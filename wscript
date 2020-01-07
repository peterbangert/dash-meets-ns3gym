## -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):
    module = bld.create_ns3_module('dash-meets-ns3gym', ['internet','config-store','stats'])
    module.includes = '.'
    module.source = [
        'model/tcp-stream-client.cc',
        'model/tcp-stream-server.cc',
        'model/tcp-stream-adaptation-algorithm.cc',
        'model/festive.cc',
        'model/panda.cc',
        'model/tobasco2.cc',
        'model/rl-algorithm.cc',
        'ns3gym/mygym.cc',
        'helper/tcp-stream-helper.cc',
        ]

    headers = bld(features='ns3header')
    headers.module = 'dash-meets-ns3gym'
    headers.source = [
        'model/tcp-stream-client.h',
        'model/tcp-stream-server.h',
        'model/tcp-stream-interface.h',
        'model/tcp-stream-adaptation-algorithm.h',
        'model/festive.h',
        'model/panda.h',
        'model/tobasco2.h',
        'model/rl-algorithm.h',
        'ns3gym/mygym.h',
        'helper/tcp-stream-helper.h',
        ]

    if bld.env['ENABLE_EXAMPLES']:
        bld.recurse('examples')

    bld.ns3_python_bindings()
