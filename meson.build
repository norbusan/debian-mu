## Copyright (C) 2021 Dirk-Jan C. Binnema <djcb@djcbsoftware.nl>
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software Foundation,
## Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

################################################################################
# project setup
#
project('mu', ['c', 'cpp'],
	version: '1.6.10',
	meson_version: '>= 0.52.0', # debian 10
	license: 'GPL-3.0-or-later',
	default_options : [
	  'buildtype=debugoptimized',
	  'warning_level=1',
	  'c_std=c11',
	  'cpp_std=c++14'
	]
       )

# installation paths
prefixdir = get_option('prefix')
bindir	  = join_paths(prefixdir, get_option('bindir'))
datadir	  = join_paths(prefixdir, get_option('datadir'))
mandir	  = join_paths(prefixdir, get_option('mandir'))
infodir	  = join_paths(prefixdir, get_option('infodir'))

################################################################################


################################################################################
# compilers / flags
#
extra_flags = [
  '-Wundef',
  '-Wwrite-strings',
  '-Wformat',
  '-Wformat-nonliteral',
  '-Wformat-security',
  '-Winit-self',
  '-Wmissing-include-dirs',
  '-Wpointer-arith',
  #'-Wswitch-enum',
  '-Wswitch-default',
]

# compilers
cc = meson.get_compiler('c')
cxx= meson.get_compiler('cpp')

# extra arguments, if available
foreach extra_arg : extra_flags
  if cc.has_argument (extra_arg)
    add_project_arguments([extra_arg], language: 'c')
  endif
  if cxx.has_argument (extra_arg)
    add_project_arguments([extra_arg], language: 'cpp')
  endif

endforeach
################################################################################



################################################################################
# config.h setup
#
config_h_data=configuration_data()
config_h_data.set_quoted('MU_STORE_SCHEMA_VERSION', '452')
config_h_data.set_quoted('PACKAGE_VERSION', meson.project_version())
config_h_data.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())
config_h_data.set_quoted('VERSION', meson.project_version())
config_h_data.set_quoted('PACKAGE_NAME', meson.project_name())

functions=[
  'setsid'
]
foreach f : functions
  if cc.has_function(f)
    define = 'HAVE_' + f.underscorify().to_upper()
    config_h_data.set(define, 1)
  endif
endforeach
################################################################################


if cc.has_function('wordexp')
  config_h_data.set('HAVE_WORDEXP_H',1)
endif


################################################################################
# hard dependencies
#
glib_dep       = dependency('glib-2.0', version: '>= 2.50')
gobject_dep    = dependency('gobject-2.0', version: '>= 2.50')
gio_dep        = dependency('gio-2.0', version: '>= 2.50')
gmime_dep      = dependency('gmime-3.0', version: '>= 3.2')
xapian_dep     = dependency('xapian-core', version:'>= 1.4')
thread_dep     = dependency('threads')

awk=find_program(['gawk', 'awk'])

# soft dependencies

# emacs -- needed for mu4e compilation
emacs=find_program(['emacs'], version: '>=25.3', required:false)
if not emacs.found()
  message('emacs not found; not pre-compiling mu4e sources')
endif

makeinfo=find_program(['makeinfo'], required:false)
if not makeinfo.found()
  message('makeinfo (texinfo) not found; not building info documentation')
endif

# readline. annoyingly, macos has incompatible libedit claiming to be readline.
# this only a dev/debug convenience for the mu4e repl.
readline_dep=[]
if get_option('readline').enabled()
  readline_dep = dependency('readline', version:'>= 8.0')
  config_h_data.set('HAVE_LIBREADLINE', 1)
  config_h_data.set('HAVE_READLINE_READLINE_H', 1)
  config_h_data.set('HAVE_READLINE_HISTORY', 1)
  config_h_data.set('HAVE_READLINE_HISTORY_H', 1)
endif

# guile
guile_deps=[]
if get_option('guile').enabled()
  guile_dep = dependency('guile-3.0')
endif

# toys.
gtk_dep=[]
webkit_dep=[]
if get_option('toys').enabled()
  gtk_dep = dependency('gtk+-3.0')
  webkit_dep = dependency('webkit2gtk-4.0')
endif
################################################################################


################################################################################
# write-out config. h.
configure_file(output : 'config.h', configuration : config_h_data)
add_project_arguments(['-DHAVE_CONFIG_H'], language: 'c')
add_project_arguments(['-DHAVE_CONFIG_H'], language: 'cpp')
config_h_dep=declare_dependency(
  include_directories: include_directories(['.']))
################################################################################



################################################################################
# write out version.texi (for texiinfo builds in mu4e, guile)
version_texi_data=configuration_data()
version_texi_data.set('VERSION', meson.project_version())
version_texi_data.set('EDITION', meson.project_version())
version_texi_data.set('UPDATED', run_command('date', '+%d %B %Y').stdout().strip())
version_texi_data.set('UPDATEDMONTH', run_command('date', '+%B %Y').stdout().strip())

configure_file(input: 'version.texi.in',
	       output: 'version.texi',
	       configuration: version_texi_data)
################################################################################


################################################################################
# subdirs
subdir('lib')
subdir('mu')
subdir('man')

if emacs.found()
   subdir('mu4e')
endif

if get_option('guile').enabled()
  subdir('guile')
endif

if get_option('toys').enabled()
  subdir('toys/mug')
endif
################################################################################
