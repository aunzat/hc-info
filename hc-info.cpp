#include <codecvt>
#include <iomanip>
#include <iostream>
#include <locale>

#include <hc.hpp>

namespace {
std::wostream& operator<<(std::wostream& out, hc::access_type type)
{
  switch (type) {
    case hc::access_type_auto:
      out << "auto";
      break;
    case hc::access_type_none:
      out << "none";
      break;
    case hc::access_type_read:
      out << "read";
      break;
    case hc::access_type_write:
      out << "write";
      break;
    case hc::access_type_read_write:
      out << "read write";
      break;
  }

  return out;
}

std::wostream& operator<<(std::wostream& out, hc::hcAgentProfile profile)
{
  switch (profile) {
    case hc::hcAgentProfileBase:
      out << "base";
      break;
    case hc::hcAgentProfileFull:
      out << "full";
      break;
    case hc::hcAgentProfileNone:
      out << "none";
      break;
  }
  return out;
}

std::wstring double_precision_str(const hc::accelerator& accelerator)
{
  if (accelerator.get_supports_double_precision()) {
    return L"yes";
  }
  if (accelerator.get_supports_limited_double_precision()) {
    return L"limited";
  }

  return L"none";
}

std::wstring bool_str(bool value)
{
  return value ? L"true": L"false";
}

std::wstring version_string(const hc::accelerator& acc)
{
  unsigned int version {acc.get_version()};
  unsigned minor { version & ((1 << 16) -1) };
  unsigned major { version >> 16 };

  return std::to_wstring(major) + L'.' + std::to_wstring(minor);
}

std::wostream& operator<<(std::wostream& out, const hc::accelerator acc)
{
  auto setw = [](){ return std::setw(25); };
  out << std::left;
  out << setw() << "device path:" << acc.get_device_path() << '\n';
  out << setw() << "description:" << acc.get_description() << '\n';
  out << setw() << "version:" << version_string(acc) << '\n';
  out << setw() << "has display:" << bool_str(acc.get_has_display()) << '\n';
  out << setw() << "dedicated memory:" << acc.get_dedicated_memory() << " KB\n";
  out << setw() << "double precision:" << double_precision_str(acc) << '\n';
  out << setw() << "debug:" << bool_str(acc.get_is_debug()) << '\n';
  out << setw() << "emulated:" << bool_str(acc.get_is_emulated()) << '\n';
  out << setw() << "CPU shared memory:"<< acc.get_supports_cpu_shared_memory() << '\n';

  out << setw() << "access type:" << acc.get_default_cpu_access_type() << '\n';
  out << setw() << "HSA Profile:" << acc.get_profile() << '\n';

  out << setw() << "peers:";
  const auto peers = acc.get_peers();
  std::for_each(std::begin(peers), std::end(peers),
      [&out](const hc::accelerator& p) { out << p.get_device_path() << " "; });
  if (peers.empty()) {
    out << '-';
  }
  out << '\n';

  return out;
}

void display_usage(std::ostream& out=std::cout)
{
  out << "usage: hc-info [DEVICE-PATH]\n";
  out << '\n';
  out << "prints information about the available HSA devices for the HC API\n";
  out << "see: https://github.com/RadeonOpenCompute/hcc\n";
  out << '\n';
  out << "HCC version: " << __hcc_version__ << '\n';
}

} // namespace 

int main(int argc, char* argv[])
{
  if (argc > 2) {
    display_usage(std::cerr);
    return EXIT_FAILURE;
  }


  if (2 == argc) {
    std::string path = argv[1];
    if ("-h" == path || "--help" == path) {
      display_usage();
      return EXIT_SUCCESS;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>> u8towide("Error");

    std::wstring wpath = u8towide.from_bytes(path);
    try {
      std::wcout << hc::accelerator(wpath);
    } catch (...) {
      std::cerr << "error getting accelerator for path: " << path << '\n';
      return EXIT_FAILURE;
    }
  } else {
    const auto accelerators = hc::accelerator::get_all();
    if (accelerators.empty()) {
      std::cout << "no accelerators found.\n";
    } else {
      std::wcout << accelerators.size() << " available accelerators:\n\n";

      std::for_each(std::begin(accelerators), std::end(accelerators),
          [](const hc::accelerator& a) { std::wcout << a << '\n'; });

      const hc::accelerator default_accelerator;
      std::wcout << "default accelerator:\t"
        << default_accelerator.get_device_path() << '\n';
    }
  }

  return EXIT_SUCCESS;
}
