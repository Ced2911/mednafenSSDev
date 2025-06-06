#include "gdb.h"
#include <inttypes.h>
#include "plat-windows.h"

namespace
{
  constexpr bool GDB_LOG_MESSAGES = false;

  constexpr u32 MAX_REQUESTS_PER_UPDATE = 10;
  constexpr u32 MAX_PACKET_SIZE = 0x4096;
  constexpr u32 DEF_BREAKPOINT_SIZE = 64;
  constexpr bool NON_STOP_MODE = false; // broken for now, mainly useful for multi-thread debugging, which we can't really support

  auto gdbCalcChecksum(const std::string &payload) -> u8
  {
    u8 checksum = 0;
    for (char c : payload)
      checksum += c;
    return checksum;
  }

  template <typename T>
  inline auto addOrRemoveEntry(std::vector<T> &data, T value, bool shouldAdd)
  {
    if (shouldAdd)
    {
      data.push_back(value);
    }
    else
    {
      // data.removeByValue(value);
      data.erase(std::remove(data.begin(), data.end(), value), data.end());
    }
  }
}

TinyGDBServer server{};

auto TinyGDBServer::reportSignal(TinyGdbSignal sig, u64 originPC) -> bool
{
  if (!hasActiveClient || !handshakeDone)
    return true; // no client -> no error
  if (forceHalt)
    return false; // Signals can only happen while the game is running, ignore others

  pcOverride = originPC;

  forceHalt = true;
  haltSignalSent = true;
  sendSignal(sig);

  return true;
}

auto TinyGDBServer::reportWatchpoint(const Watchpoint &wp, u64 address) -> void
{
  auto orgAddress = wp.addressStartOrg + (address - wp.addressStart);
  forceHalt = true;
  haltSignalSent = true;
  sendSignal(TinyGdbSignal::TRAP, wp.getTypePrefix() + hex(orgAddress) + ";");
}

auto TinyGDBServer::reportMemRead(u64 address, u32 size) -> void
{
  if (watchpointRead.empty())
    return;

  if (hooks.normalizeAddress)
  {
    address = hooks.normalizeAddress(address);
  }

  u64 addressEnd = address + size - 1;
  for (const auto &wp : watchpointRead)
  {
    if (wp.hasOverlap(address, addressEnd))
    {
      return reportWatchpoint(wp, address);
    }
  }
}

auto TinyGDBServer::reportMemWrite(u64 address, u32 size) -> void
{
  if (watchpointWrite.empty())
    return;

  if (hooks.normalizeAddress)
  {
    address = hooks.normalizeAddress(address);
  }

  u64 addressEnd = address + size - 1;
  for (const auto &wp : watchpointWrite)
  {
    if (wp.hasOverlap(address, addressEnd))
    {
      return reportWatchpoint(wp, address);
    }
  }
}

auto TinyGDBServer::reportPC(u64 pc) -> bool
{
  if (!hasActiveClient)
    return true;

  currentPC = pc;
  bool needHalts = forceHalt || vector_contains(breakpoints, pc);

  if (needHalts)
  {
    forceHalt = true; // breakpoints may get deleted after a signal, but we have to stay stopped

    if (!haltSignalSent)
    {
      haltSignalSent = true;
      sendSignal(TinyGdbSignal::TRAP);
    }
  }

  if (singleStepActive)
  {
    singleStepActive = false;
    forceHalt = true;
  }

  return !needHalts;
}

/**
 * NOTE: please read the comment in the header server.hpp file before making any changes here!
 */
auto TinyGDBServer::processCommand(const std::string &cmd, bool &shouldReply) -> std::string
{
  auto cmdParts = string_split(cmd, ':');
  auto cmdName = cmdParts[0];
  char cmdPrefix = cmdName.size() > 0 ? cmdName.at(0) : ' ';

  if constexpr (GDB_LOG_MESSAGES)
  {
    printf("GDB <: %s\n", cmdBuffer.data());
  }

  switch (cmdPrefix)
  {
  case '!':
    return "OK"; // informs us that "extended remote-debugging" is used

  case '?': // handshake: why did we halt?
    haltProgram();
    haltSignalSent = true;
    return "T05"; // needs to be faked, otherwise the GDB-client hangs up and eats 100% CPU

  case 'c': // continue
  case 'C': // continue (with signal, signal itself can be ignored)
    // normal stop-mode is only allowed to respond once a signal was raised, non-stop must return OK immediately
    handshakeDone = true; // good indicator that GDB is done, also enables exception sending
    shouldReply = NON_STOP_MODE;
    resumeProgram();
    return "OK";

  case 'D': // client wants to detach (Note: VScode doesn't seem to use this, uses vKill instead)
    requestDisconnect = true;
    return "OK";
    break;

  case 'g': // dump all general registers
    if (hooks.regReadGeneral)
    {
      return hooks.regReadGeneral();
    }
    else
    {
      return "0000000000000000000000000000000000000000";
    }
    break;

  case 'G': // set all general registers
    if (hooks.regWriteGeneral)
    {
      hooks.regWriteGeneral(cmd.substr(1));
      return "OK";
    }
    break;

  case 'H': // set which thread a 'c' command that may follow belongs to (can be ignored in stop-mode)
    if (cmdName == "Hc0")
      currentThreadC = 0;
    if (cmdName == "Hc-1")
      currentThreadC = -1;
    return "OK";

  case 'k': // old version of vKill
    if (handshakeDone)
    { // sometimes this gets send during handshake (to reset the program?) -> ignore
      requestDisconnect = true;
    }
    return "OK";
    break;

  case 'm': // read memory (e.g.: "m80005A00,4")
  {
    if (!hooks.read)
    {
      return "";
    }

    auto sepIdxMaybe = cmdName.find(",");
    u32 sepIdx = (sepIdxMaybe != std::string::npos) ? sepIdxMaybe : 1;

    u64 address = toHex(cmdName.substr(1, sepIdx - 1).c_str());
    u64 count = toHex(cmdName.substr(sepIdx + 1, cmdName.size() - sepIdx).c_str());
    return hooks.read(address, count);
  }
  break;

  case 'M': // write memory (e.g.: "M801ef90a,4:01000000")
  {
    if (!hooks.write)
    {
      return "";
    }

    auto sepIdxMaybe = cmdName.find(",");
    u32 sepIdx = (sepIdxMaybe != std::string::npos) ? sepIdxMaybe : 1;

    u64 address = toHex(cmdName.substr(1, sepIdx - 1).c_str());
    u64 byteSize = toHex(cmdName.substr(sepIdx + 1).c_str());

    std::string hexvalue = cmdParts.size() > 1 ? cmdParts[1] : "";
    std::vector<u8> value;
    std::string hexbyte;
    for (int i = 0; i < hexvalue.size(); i += 2)
    {
      std::string hexbyte = hexvalue.substr(i, 2);
      value.push_back(static_cast<u8>(toHex(hexbyte.c_str())));
    }
    hooks.write(address, value);
    return "OK";
  }

  break;

  case 'p': // read specific register (e.g.: "p15")
    if (hooks.regRead)
    {
      u32 regIdx = toInteger(cmdName.substr(1).c_str());
      return hooks.regRead(regIdx);
    }
    else
    {
      return "00000000";
    }
    break;

  case 'P': // write specific register (e.g.: "P15=FFFFFFFF80001234")
    if (hooks.regWrite)
    {
      auto sepIdxMaybe = cmdName.find("=");
      u32 sepIdx = (sepIdxMaybe != std::string::npos) ? sepIdxMaybe : 1;

      std::string regStr = cmdName.substr(1, sepIdx - 1);
      std::string valueStr = cmdName.substr(sepIdx + 1);

      u32 regIdx = static_cast<u32>(toHex(regStr.c_str()));
      u64 regValue = toHex(valueStr.c_str());

      return hooks.regWrite(regIdx, regValue) ? "OK" : "E00";
    }
    break;

  case 'q':
    // This tells the client what we can and can't do
    if (cmdName == "qSupported")
    {
      return "PacketSize=" + hex(MAX_PACKET_SIZE) +
             ";fork-events-;swbreak+;hwbreak-" +
             ";vContSupported-" +
             (NON_STOP_MODE ? ";QNonStop+" : "") +
             "QStartNoAckMode+" +
             (hooks.targetXML ? ";xmlRegisters+;qXfer:features:read+" : "");
    }

    // handshake-command, most return dummy values to convince gdb to connect
    if (cmdName == "qTStatus")
      return forceHalt ? "T1" : "";
    if (cmdName == "qAttached")
      return "1"; // we are always attached, since a game is running
    if (cmdName == "qOffsets")
      return "Text=0;Data=0;Bss=0";

    if (cmdName == "qSymbol")
      return "OK"; // client offers us symbol-names -> we don't care

    // client asks us about existing breakpoints (may happen after a re-connect) -> ignore since we clear them on connect
    if (cmdName == "qTfP")
      return "";
    if (cmdName == "qTsP")
      return "";

    // extended target features (gdb extension), most return XML data
    if (cmdName == "qXfer" && cmdParts.size() > 4)
    {
      if (cmdParts[1] == "features" && cmdParts[2] == "read")
      {
        // informs the client about arch/registers (https://sourceware.org/gdb/onlinedocs/gdb/Target-Description-Format.html#Target-Description-Format)
        if (cmdParts[3] == "target.xml")
        {
          return hooks.targetXML ? ("l" + hooks.targetXML()) : "";
        }
      }
    }

    // Thread-related queries
    if (cmdName == "qfThreadInfo")
      return {"m1"};
    if (cmdName == "qsThreadInfo")
      return {"l"};
    if (cmdName == "qThreadExtraInfo,1")
      return ""; // ignoring this command fixes support for CLion (and VSCode?), otherwise gdb hangs
    if (cmdName == "qC")
      return {"QC1"};
    // there will also be a "qP0000001f0000000000000001" command depending on the IDE, this is ignored to prevent GDB from hanging up
    break;

  case 'Q':
    if (cmdName == "QNonStop")
    { // 0=stop, 1=non-stop-mode (this allows for async GDB-communication)
      if (cmdParts.size() <= 1)
        return "E00";
      nonStopMode = cmdParts[1] == "1";

      if (nonStopMode)
      {
        haltProgram();
      }
      else
      {
        resumeProgram();
      }
      return "OK";
    }

    if (cmdName == "QStartNoAckMode")
    {
      if (noAckMode)
      {
        return "OK";
      }
      // The final OK has to be sent in ack mode.
      sendPayload("OK");
      shouldReply = false;
      noAckMode = true;
      return "";
    }
    break;

  case 's':
  {
    if (cmdName.size() > 1)
    {
      u64 address = toInteger(cmdName.substr(1).c_str());
      printf("stepping at address unsupported, ignore (%016" PRIX64 ")\n", address);
    }

    shouldReply = false;
    singleStepActive = true;
    resumeProgram();
    return "";
  }
  break;

  case 'v':
  {
    // normalize (e.g. "vAttach;1" -> "vAttach")
    auto sepIdx = cmdName.find(";");
    auto vName = (sepIdx != std::string::npos) ? cmdName.substr(0, sepIdx) : cmdName;

    if (vName == "vMustReplyEmpty")
      return ""; // handshake-command / keep-alive (must return the same as an unknown command would)
    if (vName == "vAttach")
      return NON_STOP_MODE ? "OK" : "S05"; // attaches to the process, we must return a fake trap-exception to make gdb happy
    if (vName == "vCont?")
      return ""; // even though "vContSupported-" is set, gdb may still ask for it -> ignore to force e.g. `s` instead of `vCont;s:1;c`
    if (vName == "vStopped")
      return "";
    if (vName == "vCtrlC")
    {
      haltProgram();
      return "OK";
    }

    if (vName == "vKill")
    {
      if (handshakeDone)
      { // sometimes this gets send during handshake (to reset the program?) -> ignore
        requestDisconnect = true;
      }
      return "OK";
    }

    if (vName == "vCont")
      return "E00"; // if GDB completely ignores both "vCont is unsupported" responses, throw an error here
  }
  break;

  case 'Z': // insert breakpoint (e.g. "Z0,801a0ef4,4")
  case 'z': // remove breakpoint (e.g. "z0,801a0ef4,4")
  {
    bool isInsert = cmdPrefix == 'Z';
    bool isHardware = cmdName.at(1) == '1';  // 0=software, 1=hardware
    auto sepIdxMaybe = cmdName.find(",", 3); // Recherche à partir de l'indice 3
    u32 sepIdx = (sepIdxMaybe != std::string::npos) ? (sepIdxMaybe) : 0;

    u64 address = toHex(cmdName.substr(3, sepIdx - 1).c_str());
    u64 addressStart = address;
    u64 addressEnd = address + toHex(cmdName.substr(sepIdx + 1).c_str()) - 1;

    if (hooks.normalizeAddress)
    {
      addressStart = hooks.normalizeAddress(addressStart);
      addressEnd = hooks.normalizeAddress(addressEnd);
    }
    Watchpoint wp{addressStart, addressEnd, address};

    switch (cmdName.at(1))
    {
    case '0': // (hardware/software breakpoints are the same for us)
    case '1':
      addOrRemoveEntry(breakpoints, address, isInsert);
      break;

    case '2':
      wp.type = WatchpointType::WRITE;
      addOrRemoveEntry(watchpointWrite, wp, isInsert);
      break;

    case '3':
      wp.type = WatchpointType::READ;
      addOrRemoveEntry(watchpointRead, wp, isInsert);
      break;

    case '4':
      wp.type = WatchpointType::ACCESS;
      addOrRemoveEntry(watchpointRead, wp, isInsert);
      addOrRemoveEntry(watchpointWrite, wp, isInsert);
      break;
    default:
      return "E00";
    }

    if (hooks.emuCacheInvalidate)
    { // for re-compiler, otherwise breaks might be skipped
      hooks.emuCacheInvalidate(address);
    }
    return "OK";
  }
  }

  printf("Unknown-Command: %s (data: %s)\n", cmdName.data(), cmdBuffer.data());
  return "";
}

auto TinyGDBServer::onText(std::string_view text) -> void
{

  if (cmdBuffer.size() == 0)
  {
    cmdBuffer.reserve(text.size());
  }

  for (char c : text)
  {
    switch (c)
    {
    case '$':
      insideCommand = true;
      break;

    case '#':
    { // end of message + 2-char checksum after that
      insideCommand = false;

      ++messageCount;
      bool shouldReply = true;
      auto cmdRes = processCommand(cmdBuffer, shouldReply);
      if (shouldReply)
      {
        sendPayload(cmdRes);
      }
      else if (!noAckMode)
      {
        sendText("+");
      }

      cmdBuffer = "";
    }
    break;

    case '+':
      break; // "OK" response -> ignore

    case '\x03': // CTRL+C (same as "vCtrlC" packet) -> force halt
      if constexpr (GDB_LOG_MESSAGES)
      {
        printf("GDB <: CTRL+C [0x03]\n");
      }
      haltProgram();
      break;

    default:
      if (insideCommand)
      {
        // cmdBuffer.append(c);
        cmdBuffer += c;
      }
    }
  }
}

auto TinyGDBServer::updateLoop() -> void
{
  if (!isStarted())
    return;

  if (requestDisconnect)
  {
    requestDisconnect = false;
    if (!noAckMode)
    {
      sendText("+");
    }
    disconnectClient();
    resumeProgram();
    return;
  }

  // The following code manages the message processing which gets exchanged from the server thread.
  // It was carefully build to balance latency, throughput and CPU usage to let the game still run at full speed
  // while allowing for fast processing once the debugger is halted.

  u32 loopFrames = isHalted() ? 20 : 1;   // "frames" to check (loops with sleep in-between)
  u32 loopCount = isHalted() ? 500 : 100; // loops inside a frame, the more the less latency, but CPU usage goes up
  u32 maxLoopResets = 10000;              // how many times can a new message reset the counter (prevents infinite loops with misbehaving clients)
  bool wasHalted = isHalted();

  for (u32 frame = 0; frame < loopFrames; ++frame)
  {
    for (u32 i = 0; i < loopCount; ++i)
    {
      messageCount = 0;
      update();

      // if the last message resumed the program, abort (no more messages will be send until the next stop)
      if (wasHalted && !isHalted())
        return;

      if (messageCount > 0 && maxLoopResets > 0)
      {
        i = loopCount; // reset loop here to keep a fast chain of messages going (reduces latency)
        --maxLoopResets;
      }
    }

    if (wasHalted)
      usleep(1);
  }
}

auto TinyGDBServer::getStatusText(u32 port, bool useIPv4) -> std::string
{
  auto url = getURL(port, useIPv4);
  std::string prefix = isHalted() ? "⬛" : "▶";

  if (hasClient())
    return prefix + " GDB connected " + url;
  if (isStarted())
    return "GDB listening " + url;
  return "GDB pending (" + url + ")";
}

auto TinyGDBServer::sendSignal(TinyGdbSignal code) -> void
{
  sendPayload("S" + hex(static_cast<u8>(code), 2));
}

auto TinyGDBServer::sendSignal(TinyGdbSignal code, const std::string &reason) -> void
{
  sendPayload("T" + hex(static_cast<u8>(code), 2) + reason);
}

auto TinyGDBServer::sendPayload(const std::string &payload) -> void
{
  std::string msg = (noAckMode ? "$" : "+$") + payload + '#' + hex(gdbCalcChecksum(payload), 2, '0');

  if constexpr (GDB_LOG_MESSAGES)
  {
    printf("GDB >: %.*s\n", msg.size() > 100 ? 100 : msg.size(), msg.data());
  }
  sendText(msg);
}

auto TinyGDBServer::haltProgram() -> void
{
  forceHalt = true;
  haltSignalSent = false;
}

auto TinyGDBServer::resumeProgram() -> void
{
  pcOverride.reset();
  forceHalt = false;
  haltSignalSent = false;
}

auto TinyGDBServer::onConnect() -> void
{
  printf("GDB client connected\n");
  resetClientData();
  hasActiveClient = true;
}

auto TinyGDBServer::onDisconnect() -> void
{
  if (hasActiveClient)
    printf("GDB client disconnected\n");
  hadHandshake = false;
  resetClientData();
}

auto TinyGDBServer::reset() -> void
{
  hooks.read = nullptr;
  hooks.write = nullptr;
  hooks.normalizeAddress = nullptr;
  hooks.regReadGeneral = nullptr;
  hooks.regWriteGeneral = nullptr;
  hooks.regRead = nullptr;
  hooks.regWrite = nullptr;
  hooks.emuCacheInvalidate = nullptr;
  hooks.targetXML = nullptr;

  resetClientData();
}

auto TinyGDBServer::resetClientData() -> void
{
  breakpoints.clear();
  breakpoints.reserve(DEF_BREAKPOINT_SIZE);

  watchpointRead.clear();
  watchpointRead.reserve(DEF_BREAKPOINT_SIZE);

  watchpointWrite.clear();
  watchpointWrite.reserve(DEF_BREAKPOINT_SIZE);

  pcOverride.reset();
  insideCommand = false;
  cmdBuffer = "";
  haltSignalSent = false;
  forceHalt = false;
  singleStepActive = false;
  nonStopMode = false;
  noAckMode = false;

  currentThreadC = -1;
  hasActiveClient = false;
  handshakeDone = false;
  requestDisconnect = false;
}
