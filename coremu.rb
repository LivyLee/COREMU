require 'pty'
require 'expect'
$expect_verbose = true

class COREMU
  @@qemu_x86 = 'obj/qemu/x86_64-softmmu/qemu-system-x86_64'
  #@@qemu_x86 = '../cm-qemu/x86_64-softmmu/qemu-system-x86_64'

  @@corey_dir = "~/related-coremu/mit-corey/obj"
  @@corey = {
    :executable => @@qemu_x86,
    :hda => "#{@@corey_dir}/boot/bochs.img",
    :hdb => "#{@@corey_dir}/fs/fs.fat",
    :memsize => 1024,
  }

  @@linux_dir = "~/linux-img"
  @@linux = {
    :executable => @@qemu_x86,
    #:hda => "#{@@linux_dir}/debian-bench.img",
    :hda => "#{@@linux_dir}/debian6.img",
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

  def self.gen_cmd(conf, mode, core)
    cmd = "sudo #{conf[:executable]} " \
      "-k en-us " \
      "-serial mon:/dev/tty " \
      "-nographic " \
      "-net none " \
      "-smp #{core} " \
      #"-d in_asm "
    cmd << "-m #{conf[:memsize]} " if conf.has_key? :memsize
    cmd << "-hda #{conf[:hda]}.#{mode} " if conf.has_key? :hda
    cmd << "-hdb #{conf[:hdb]}.#{mode} " if conf.has_key? :hdb
    cmd << "-kernel #{conf[:kernel]} " if conf.has_key? :kernel
    cmd << "-initrd #{conf[:initrd]} " if conf.has_key? :initrd
    cmd << "-runmode #{mode} " if mode != :normal
    cmd << conf[:addition] if conf.has_key? :addition
    cmd
  end

  def self.corey_cmd(core, mode = :normal)
    c = gen_cmd(@@corey, mode, core)
    puts c
    c
  end

  def self.setup_corey(mode)
    create_qcow2_disk @@corey[:hda], mode
    create_qcow2_disk @@corey[:hdb], mode
  end

  def self.run_corey(core, mode = :normal)
    setup_corey mode
    exec corey_cmd(core, mode)
  end

  def self.run_corey_autoexit(core, mode)
    setup_corey mode
    puts corey_cmd(core, mode)
    PTY.spawn(corey_cmd(core, mode)) do |reader, writer, pid|
      reader.expect(/thread_halt: no more threads/) do |r|
        writer.printf("?\C-ax")
      end

      reader.expect(".*")
    end
  end

  def self.linux_cmd(core, mode = :normal)
    c = gen_cmd(@@linux, mode, core) + "-bios bin/share/qemu/seabios.bin "
    puts c
    c
  end

  def self.setup_linux(mode = :normal)
    create_qcow2_disk @@linux[:hda], mode
  end

  def self.run_linux(core, mode = :normal)
    setup_linux mode
    exec linux_cmd(core, mode)
  end

  @@qemu_arm = 'obj/qemu/arm-softmmu/qemu-system-arm'
  @@arm_dir = "~/linux-img/arm"
  @@arm = {
    :executable => @@qemu_arm,
    :kernel => "#{@@arm_dir}/linux-2.6.28/arch/arm/boot/zImage",
    :initrd => "#{@@arm_dir}/initrd.gz",
    :memsize => 128,
    :additional_option => "-M realview-pbx-a9 -net none ",
  }

  def self.arm_cmd(core, mode = :normal)
    cmd = gen_cmd(@@arm, mode, core)
    puts cmd
    cmd
  end

  def self.run_arm(core, mode = :normal)
    exec arm_cmd(core, mode)
  end

  def self.run_benchmark(cmd, exp_str, logfile)
    PTY.spawn(cmd) do |reader, writer, pid|
      reader.expect(exp_str) do |r|
	writer.printf("?\C-ax")
	logfile.puts(r[1])
	puts r[1] if logfile != STDIN
      end
    end
  end

  BENCHMARK_NTIMES = 3
  def self.benchmark_linux(exp_str, mode = :normal)
    if ARGV.length != 2
      puts "Usage: #{$0} #cores logfile"
      exit 1
    end

    ncore = ARGV[0]
    logpath = ARGV[1]

    setup_linux mode
    cmd = linux_cmd(ncore, mode)

    logfile = (logpath == "stdin" ? STDIN : File.open(logpath, 'w'))

    BENCHMARK_NTIMES.times do
      run_benchmark(cmd, exp_str, logfile)
      sleep 1
    end

    logfile.close if logfile != "stdin"
  end
end

