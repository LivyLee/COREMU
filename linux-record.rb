#!/usr/bin/env ruby

require 'coremu'

COREMU.check_args

COREMU.run_linux 'record', ARGV[0]
