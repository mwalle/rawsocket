#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os

# pip monkeypatches setup.py to always use setuptools. Unfortunately, this does
# not work if we subclass our install command from
# distutils.command.install.install. Therefore, we try to import the setuptools
# package first.
try:
    from setuptools import setup, Extension, Command
    from setuptools.command.install import install
except ImportError:
    from distutils.core import setup, Extension, Command
    from distutils.command.install import install

from distutils.core import setup, Extension, Command
from distutils.command.build import build
from distutils.command.clean import clean
from distutils import ccompiler, log
from distutils.dir_util import remove_tree

name = 'rawsocket'
with open('README.rst') as f:
    readme = f.read()
version = '0.1'

mod_rawsocket = Extension(
    name = 'rawsocket',
    sources = ['rawsocket.c'],
    define_macros = [('VERSION', '"%s"' % version)],
)

class run_build_helper(Command):
    description = 'Build helper binary'
    user_options = []

    def initialize_options(self):
        self.plat_name = None
        self.build_helper = None
        self.build_base = None
        self.build_temp = None

    def finalize_options(self):
        self.set_undefined_options('build',
                ('build_base', 'build_base'),
                ('build_temp', 'build_temp'),
                ('plat_name', 'plat_name'),
        )
        plat_specifier = '.%s-%s' % (self.plat_name, sys.version[0:3])
        self.build_helper = os.path.join(self.build_base,
                'helper' + plat_specifier)

    def build(self, name, sources):
        log.info("building '%s' helper", name);
        compiler = ccompiler.new_compiler()
        objects = compiler.compile(sources, output_dir=self.build_temp)
        filename = os.path.join(self.build_helper, name)
        compiler.link_executable(objects, filename)

    def run(self):
        self.build('rawsocket-helper', ['rawsocket-helper.c'])

class my_build(build):
    def run(self):
        self.run_command('build_helper')
        build.run(self)

class my_install(install):
    def initialize_options(self):
        install.initialize_options(self)
        self.build_helper = None

    def finalize_options(self):
        install.finalize_options(self)
        self.set_undefined_options('build_helper',
                ('build_helper', 'build_helper'),
        )

    def run(self):
        self.run_command('build_helper')
        if os.path.isdir(self.build_helper):
            self.copy_tree(self.build_helper, self.install_scripts)
        install.run(self)

class my_clean(clean):
    def initialize_options(self):
        clean.initialize_options(self)
        self.build_helper = None

    def finalize_options(self):
        clean.finalize_options(self)
        self.set_undefined_options('build_helper',
                ('build_helper', 'build_helper'),
        )

    def run(self):
        if os.path.exists(self.build_helper):
            remove_tree(self.build_helper, dry_run=self.dry_run)
        clean.run(self)

setup(name = name,
        version = version,
        description = 'Raw packet socket for Linux',
        long_description = readme,
        author = 'Michael Walle',
        author_email = 'michael@walle.cc',
        url = 'http://github.com/mwalle/rawsocket',
        license = 'New BSD',
        classifiers = [
            'Development Status :: 4 - Beta',
            'License :: OSI Approved :: BSD License',
            'Natural Language :: English',
            'Operating System :: POSIX :: Linux',
            'Programming Language :: Python :: 2',
            'Programming Language :: Python :: 2.7',
            'Topic :: Software Development :: Libraries :: Python Modules',
            'Topic :: System :: Networking',
        ],
        ext_modules = [ mod_rawsocket ],
        cmdclass = {
            'build': my_build,
            'clean': my_clean,
            'install': my_install,
            'build_helper': run_build_helper,
        },
)
