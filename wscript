#!/usr/bin/env python

import os, platform, subprocess, sys
from os.path import expanduser

sys.path.insert (0, "tools/waf")
import cef

def native_ui_type():
    if 'darwin' in sys.platform:  return 'CocoaUI'
    elif 'linux' in sys.platform:   return 'X11UI'
    elif 'windows' in sys.platform: return 'WindowsUI'
    else: return ''

def platform_pre_config (conf):
    if sys.platform == 'darwin':
        if not conf.env.CC or not conf.env.CXX:
            conf.env.CC  = 'clang'
            conf.env.CXX = 'clang++'
        conf.env.ARCH_COCOA = ['i386', 'x86_64']
        conf.env.CFLAGS_MACOSX    = ['-arch', 'i386', '-arch', 'x86_64']
        conf.env.CXXFLAGS_MACOSX  = ['-arch', 'i386', '-arch', 'x86_64']
        conf.env.LINKFLAGS_MACOSX = ['-arch', 'i386', '-arch', 'x86_64']

def options (opt):
    opt.load ('compiler_c compiler_cxx')
    opt.add_option('--lv2user', default=False, action='store_true', \
                    dest="lv2user", help='Install Plugins to the LV2 User location')
    opt.add_option('--debug', default=False, action='store_true', \
                    dest='debug', help="Enable Compiler debug flags")
    opt.add_option('--debug-lvtk', default=False, action='store_true', \
                    dest='debug_lvtk', help="Enable LVTK debug flags")

def configure_lv2dir (conf):
    if 'linux' in sys.platform:
        if conf.options.lv2user:
            conf.env.LV2DIR = expanduser('~') + '.lv2'
        else:
            conf.env.LV2DIR = conf.env.PREFIX + '/lib/lv2'
    elif 'linux' in sys.platform:
        if conf.options.lv2user:
            conf.env.LV2DIR = expanduser('~') + '/Library/Audio/Plug-Ins/LV2'
        else:
            conf.env.LV2DIR = '/Library/Audio/Plug-Ins/LV2'
    elif 'indows' in sys.platform:
        pass

def configure (conf):
    platform_pre_config (conf)
    conf.load ('compiler_c compiler_cxx')

    # Setup installation directories
    configure_lv2dir (conf)

    conf.env['INCLUDEDIR'] = os.path.join (conf.env.PREFIX, 'include')
    conf.env['LIBDIR']     = conf.env.PREFIX + '/lib'
    conf.env['CEFUIDIR']   = conf.env.LV2DIR + '/cefui.lv2'

    conf.find_program ('gperf')

    conf.env.append_unique ('CXXFLAGS', ['-std=c++11'])
    conf.check (header_name=['memory'])
    conf.check (header_name=['unordered_map'])
    conf.check (header_name=['sys/capability.h'])
    conf.check (header_name=['gssapi.h'])

    conf.check_cfg (package='lv2', uselib_store='LV2', args='--cflags', version='1.8.0', mandatory=True)
    conf.check_cfg (package='lilv-0', uselib_store='LILV', args='--cflags --libs', version='0.18.1', mandatory=True)
    conf.check_cfg (package='gtk+-2.0', uselib_store='GTK', args='--cflags --libs', version='2.18.0', mandatory=True)

    pat = conf.env['cshlib_PATTERN']
    if not pat:
        pat = conf.env['cxxshlib_PATTERN']
    if pat.startswith('lib'):
        pat = pat[3:]
    conf.env['plugin_PATTERN'] = pat
    conf.env['plugin_EXT'] = pat[pat.rfind('.'):]

    conf.env['CEF_BRANCH'] = 1916
    conf.env['DEBUG_BUILD'] = conf.options.debug;

    conf.env.append_unique ('CXXFLAGS', ['-DLVTK_DEBUG=%i' % int (conf.options.debug_lvtk)])


def build_libcef (bld):
    return # disabled

    if 'build' not in bld.cmd:
        return

    command = ['python', 'tools/automate.py']
    command.append ('--branch=%s' % (bld.env.CEF_BRANCH))
    command.append ('--download-dir=' + os.path.join (os.getcwd(), 'libs'))
    command.append ('--force-build')
    #print command
    subprocess.call (command)


def post_build (bld):
    if 'build' != bld.cmd:
        return

    # Copy HTML content with rsync
    src = os.path.join (os.getcwd(), 'plugins/cefamp.lv2/content/')
    tgt = os.path.join (os.getcwd(), 'build/stage/lib/lv2/cefamp.lv2/content/')
    subprocess.call (['rsync', '-var', '--update', src, tgt])


def post_install (bld):
    if 'install' != bld.cmd:
        return
    
    '''the full path to the cefui directory'''
    cefdir = bld.options.destdir + bld.env.CEFUIDIR

    ''' Chrome sandbox needs to be owned by root with 4755 permisions
        regardless of where it is installed'''
    sandbox = os.path.join (cefdir, 'chrome-sandbox')
    if os.path.exists (sandbox):
        subprocess.call (['chmod', '4755', sandbox])

    '''Fix permissions of CEF libraries'''
    for lib in ['libcef.so', 'libffmpegsumo.so']:
        f = os.path.join (cefdir, lib)
        if os.path.exists (f):
            os.chmod (f, 0755)

def build (bld):
    # CEF must be compiled before anything else
    bld.add_pre_fun (build_libcef)
    
    glob = bld.path.ant_glob
    join = os.path.join
    debug = bld.env.DEBUG_BUILD

    # Build the support library
    bld.shlib (
        includes      = ['.'],
        source        = glob ('src/*.cpp'),
        name          = 'libcefui',
        target        = 'stage/lib/cefui-0',
        install_path  = bld.env.LIBDIR,
        use           = [],
        vnum          = '0.0.1'
    )

    # The pkg-config file
    bld (
        features     = 'subst',
        source       = 'cefui-0.pc.in',
        target       = 'stage/lib/pkgconfig/cefui-0.pc',
        install_path = join (bld.env.LIBDIR, 'pkgconfig'),
        THELIB       = 'cefui-0',
        PKGDEPS      = '',
        MAJOR_VERSION = '0',
        PREFIX       = bld.env.PREFIX,
        EXEC_PREFIX  = '',
        LIBDIR       = bld.env.LIBDIR,
        INCLUDEDIR   = bld.env.INCLUDEDIR,
        VERSION      = '0.0.1'
    )

    bld.install_files (join (bld.env.PREFIX, 'include/cefui-0/cefui'), glob ('cefui/*.h'))
    bld.add_group()

    cefresdir   = cef.get_resdir();
    ceflibdir = cef.get_libdir (debug)
    ceftopdir = 'libs/cef_binary'

    bundlename = 'cefui.lv2'
    bundledir = '%s/%s' % (bld.env.LV2DIR, bundlename)

    plugin_environ = bld.env.derive()
    plugin_environ.cshlib_PATTERN = plugin_environ.cxxshlib_PATTERN = \
        bld.env.plugin_PATTERN

    # Include misc libraries created by the chromium build
    for lib in glob (ceflibdir + '/*.so'):
        libstr = '%s' % lib
        bld (
            rule    = 'cp ${SRC} ${TGT}',
            source  = join (ceflibdir, libstr),
            target  = join ('stage/lib/lv2', bundlename, libstr),
            name    = libstr.replace ('.so', ''),
            install_path = bld.env.CEFUIDIR,
        )

    # Copy non-locale packs
    for pak in glob (cefresdir + '/*.pak'):
        bld (
            rule = 'cp ${SRC} ${TGT}',
            source = join (cefresdir, '%s' % pak),
            target = join ('stage/lib/lv2', bundlename, '%s' % pak),
            name   = '%s' % pak,
            install_path = bld.env.CEFUIDIR
         )

    # Copy data files
    for dat in glob (cefresdir + '/*.dat'):
        bld (
            rule = 'cp ${SRC} ${TGT}',
            source = join (cefresdir, '%s' % dat),
            target = join ('stage/lib/lv2', bundlename, '%s' % dat),
            name   = '%s' % dat,
            install_path = bld.env.CEFUIDIR
         )

    # Copy Locales
    for pak in glob (cefresdir + '/locales/*.pak'):
        bld (
            rule = 'cp ${SRC} ${TGT}',
            source = join (cefresdir + '/locales', '%s' % pak),
            target = join ('stage/lib/lv2', bundlename, 'locales/%s' % pak),
            name   = '%s' % pak,
            install_path = join ('%s' % bld.env.CEFUIDIR, 'locales')
         )

    bld.add_group()

    bld (
        features     = 'subst',
        source       = 'plugins/cefui.lv2/manifest.ttl',
        target       = join ('stage/lib/lv2', bundlename, 'manifest.ttl'),
        install_path = bundledir,
        LIB_EXT      = plugin_environ.plugin_EXT,
        UI_TYPE      = 'X11UI'
    )

    bld (
        features     = 'subst',
        source       = 'plugins/cefui.lv2/cefui.ttl',
        target       = join ('stage/lib/lv2', bundlename, 'cefui.ttl'),
        install_path = bundledir,
        UI_TYPE      = 'X11UI'
    )

    client_code = glob ('libs/cef_binary/libcef_dll/**/*.c*') + \
                  glob ('plugins/cefui.lv2/client/native/*_gtk.cpp') + \
                  glob ('plugins/cefui.lv2/client/*.cpp')

    # Search in the lv2 bundle for CEF related libraries
    ceflinkflags = ['-L./stage/lib/lv2/cefui.lv2', '-lcef', '-Wl,-rpath,$ORIGIN']

    # Build the LV2 UI + Helper Library
    libcefui = bld.shlib (
        includes      = [ceftopdir, '.', 'libs/lvtk', 'plugins/cefui.lv2'],
        source        = client_code,
        name          = 'cefui_client',
        target        = join ('stage/lib/lv2', bundlename, 'cefui_client'),
        install_path  = bld.env.CEFUIDIR,
        linkflags     = ceflinkflags,
        cxxflags      = cef.get_cxxflags (debug),
        cflags        = cef.get_cxxflags (debug),
        use           = ['LV2', 'LILV', 'GTK', 'libcefui'],
    )

    # Build the subprocess chromium needs for content rendering
    renderer = bld.program (
        includes      = [ceftopdir, 'plugins/cefui.lv2'],
        source        = glob ('plugins/cefui.lv2/renderer/**/*.cpp'),
        name          = 'cefui-renderer',
        target        = join ('stage/lib/lv2', bundlename, 'cefui-renderer'),
        install_path  = bundledir,
        cxxflags      = cef.get_cxxflags (debug),
        linkflags     = ceflinkflags,
        use           = ['cefui_client']
    )

    # Install the HTML/JS content
    bld.install_files (bundledir + '/content', glob ('plugins/cefui.lv2/content/**/*'), \
        cwd=bld.path.find_dir ('plugins/cefui.lv2/content'), relative_trick=True)

    # Install the chrome-sandbox
    bld.install_as (join ('%s' % bld.env.CEFUIDIR, 'chrome-sandbox'), \
                    join (ceflibdir, 'chrome-sandbox'), chmod=0755)

    bld.add_group()

    plugin_environ = bld.env.derive()
    plugin_environ.cshlib_PATTERN = plugin_environ.cxxshlib_PATTERN = \
        bld.env.plugin_PATTERN

    bundlename = 'cefamp.lv2'
    bundledir = '%s/%s' % (bld.env.LV2DIR, bundlename)

    bld (
        features     = 'subst',
        source       = 'plugins/cefamp.lv2/manifest.ttl',
        target       = join ('stage/lib/lv2', bundlename, 'manifest.ttl'),
        install_path = bundledir,
        LIB_EXT      = plugin_environ.plugin_EXT,
        UI_TYPE      = 'GtkUI'
    )

    bld (
        features     = 'subst',
        source       = 'plugins/cefamp.lv2/cefamp.ttl',
        target       = join ('stage/lib/lv2', bundlename, 'cefamp.ttl'),
        install_path = bundledir
    )

    amp = bld.shlib (
        source       = glob ('plugins/cefamp.lv2/*.cpp'),
        includes     = ['libs/lvtk', 'src'],
        name         = 'cefamp',
        target       = join ('stage/lib/lv2', bundlename, 'cefamp'),
        env          = plugin_environ,
        install_path = bundledir,
        linkflags    = [],
        cxxflags     = [],
        use          = ['LV2']
    )

    tests = bld.program (
        source       = glob ('tests/*.cpp'),
        includes     = '.',
        name         = 'tests',
        target       = 'stage/bin/tests',
        linkflags    = ['-lboost_unit_test_framework'],
        use          = ['libcefui']
    )
    bld.add_post_fun (post_build)
    bld.add_post_fun (post_install)
