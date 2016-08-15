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

std::wostream& operator<<(std::wostream& out, const hc::accelerator& acc)
{
  out << std::left;
  auto out_w = [&]() -> std::wostream& { return out << std::setw(25); };

  out_w() << "device path:" << acc.get_device_path() << '\n';
  out_w() << "description:" << acc.get_description() << '\n';
  out_w() << "version:" << version_string(acc) << '\n';

  out_w() << "dedicated memory:" << acc.get_dedicated_memory() << " KB\n";
  out_w() << "CPU shared memory:"
    << bool_str(acc.get_supports_cpu_shared_memory()) << '\n';

  out_w() << "max static tile size:"
    << const_cast<hc::accelerator&>(acc).get_max_tile_static_size() << '\n';

  out_w() << "default CPU mem access:"
    << acc.get_default_cpu_access_type() << '\n';

  out_w() << "compute unit count:" << acc.get_cu_count() << '\n';

  out_w() << "double precision:" << double_precision_str(acc) << '\n';

  out_w() << "debug:" << bool_str(acc.get_is_debug()) << '\n';
  out_w() << "has display:" << bool_str(acc.get_has_display()) << '\n';
  out_w() << "emulated:" << bool_str(acc.get_is_emulated()) << '\n';

  out_w() << "HSA Profile:" << acc.get_profile() << '\n';

  out_w() << "peers:";
  const auto peers = acc.get_peers();
  std::for_each(std::begin(peers), std::end(peers),
      [&out](const hc::accelerator& p) { out << p.get_device_path() << " "; });
  if (peers.empty()) {
    out << '-';
  }
  out << '\n';

  return out;
}

std::string backend_str(int backend)
{
  switch (backend) {
    case HCC_BACKEND_AMDGPU:
      return "HCC_BACKEND_AMDGPU";
    case HCC_BACKEND_HSAIL:
      return "HCC_BACKEND_HSAIL";
    case HCC_BACKEND_CL:
      return "HCC_BACKEND_CL";

    default:
      return "Unknown HCC Backend";
  }
}

void display_usage(std::ostream& out=std::cout)
{
  out << "usage: hc-info [DEVICE-PATH | default]\n";
  out << '\n';
  out << "prints information about the available HSA devices for the HC API\n";
  out << "see: https://github.com/RadeonOpenCompute/hcc\n";
  out << '\n';
  out << "compiled for:\n";
  out << "HCC version: " << __hcc_version__ << '\n';
  out << "Backend: " << backend_str(__hcc_backend__) << '\n';
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
      const hc::accelerator acc(wpath);
      if (wpath != L"default" && acc.get_device_path() != wpath) { // hc::acc does not throw on no dev
        throw std::runtime_error("no accelerator");
      }

      std::wcout << acc;
    } catch (...) {
      std::cerr << "no accelerator for path: '" << path << "'\n";
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
