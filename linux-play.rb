#!/usr/bin/env ruby

require 'coremu'

COREMU.check_args

COREMU.run_linux 'replay', ARGV[0]

