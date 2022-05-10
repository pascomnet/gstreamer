# CL-2812 on windows we can use OpenSSL from Qt instead of compiling it on our own
# unfortunatelly the openssl.wrap build doesn't work on windows, see: https://github.com/mesonbuild/wrapdb/issues/181

project(
  'openssl',
  'c',
  version: '1.1.1k',
  license : 'Apache-2.0',
  meson_version: '>= 0.55',
  default_options: [
    'warning_level=1',
  ],
)

if host_machine.system() == 'windows'
  include_directories = include_directories('C:\\Qt\\Tools\\OpenSSL\\Win_x64\\include')
  
  libcrypto_dep = declare_dependency(
    include_directories: include_directories,
    link_args : ['/LIBPATH:C:\\Qt\\Tools\\OpenSSL\\Win_x64\\lib', 'libcrypto.lib'],
  )
  
  libssl_dep = declare_dependency(
    include_directories: include_directories,
    link_args : ['/LIBPATH:C:\\Qt\\Tools\\OpenSSL\\Win_x64\\lib', 'libssl.lib'],
  )
  
  openssl_dep = declare_dependency(
    dependencies: [libcrypto_dep, libssl_dep],
  )
endif