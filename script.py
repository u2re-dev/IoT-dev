Import("env")
platform = env.PioPlatform()
pkg = platform.get_package("framework-espidf")
pkg.metadata.version.build = None
