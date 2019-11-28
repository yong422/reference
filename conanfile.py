#/usr/bin/env python
#-*- coding:utf-8 -*-

"""
conanfile.py
프로젝트에서 사용하는 libraries install, build
conan
"""

from conans import ConanFile, CMake

class MylibConan(ConanFile):
    setting = "os", "compiler", "build_type", "arch"
    requires = (("zlib/1.2.11@conan/stable"),\
                ("OpenSSL/1.1.1c@conan/stable"),\
                ("gtest/1.8.1@bincrafters/stable"),\
                ("google-benchmark/1.4.1@mpusz/stable"),\
                ("libcurl/7.61.1@bincrafters/stable"),\
                ("maxminddb/1.3.2@monkeber/stable"),
                ("hiredis/0.14.0@ykjo/stable"))

    generators = "cmake"

    option = {"libcurl:fPIC" : True,\
              "libcurl:with_openssl": True,\
              "maxminddb:shared": False }

    def imports(self):
         self.copy("license*", dst="licenses", folder=True, ignore_case=True)
