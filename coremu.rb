require 'pty'
require 'expect'
$expect_verbose = true

class COREMU
  @@qemu = 'obj/qemu/x86_64-softmmu/qemu-system-x86_64'
  #@@qemu = '../cm-qemu/x86_64-softmmu/qemu-system-x86_64'

  @@corey_dir = "~/coremu/mit-corey/obj"
  @@corey = {
    :hda => "#{@@corey_dir}/boot/bochs.img",
    :hdb => "#{@@corey_dir}/fs/fs.fat",
    :memsize => 1024,
  }

  @@linux_dir = "~/linux-img"
  @@linux = {
    :hda => "#{@@linux_dir}/arch.img",
    :memsize => 1024,
  }

  def self.check_args
    if ARGV.length != 1
      puts "Need to specify number of cores"
      exit 1
    end
  end

  def self.create_qcow2_disk(hd, mode)
    system "sudo qemu-img create -f qcow2 -b #{hd} #{hd}.#{mode}"
    puts "created image #{hd}.#{mode}"
  end

  def self.cmd(conf, mode, core)
    cmd = "sudo #{@@qemu} " \
      "-m #{conf[:memsize]} " \
      "-k en-us " \
      "-serial mon:/dev/tty " \
      "-nographic " \
      "-bios bin/share/qemu/seabios.bin " \
      "-net none " \
      "-smp #{core} " \
      "-hda #{conf[:hda]}.#{mode} " \
      "-runmode #{mode} " \
      #"-d in_asm " \
  end

  def self.corey_cmd(mode, core)
    c = cmd(@@corey, mode, core) + "-hdb #{@@corey[:hdb]}.#{mode}"
    puts c
    c
  end

  def self.setup_corey(mode)
    create_qcow2_disk @@corey[:hda], mode
    create_qcow2_disk @@corey[:hdb], mode
  end

  def self.run_corey(mode, core)
    setup_corey mode
    exec corey_cmd(mode, core)
  end

  def self.run_corey_autoexit(mode, core)
    setup_corey mode
    puts corey_cmd(mode, core)
    PTY.spawn(corey_cmd(mode, core)) do |reader, writer, pid|
      reader.expect(/thread_halt: no more threads/) do |r|
        writer.printf("?\C-ax")
      end

      reader.expect(".*")
    end
  end

  def self.linux_cmd(mode, core)
    c = cmd(@@linux, mode, core)
    puts c
    c
  end

  def self.setup_linux(mode)
    create_qcow2_disk @@linux[:hda], mode
  end

  def self.run_linux(mode, core)
    setup_linux mode
    exec linux_cmd(mode, core)
  end
end

