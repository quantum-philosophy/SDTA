def locate *args
  filename = File.join *args
  raise "Cannot find '#{ filename }'" unless File.exist? filename
  filename
end

def run options
  args = Array(options[:arguments])

  if options.has_key? :command
    command = "#{ options[:command] } #{ args.join ' ' }"
  else
    tool = Array(options[:tool])
    command = "#{ locate *tool } #{ args.join ' ' }"
  end

  unless options[:output].nil?
    command = "#{ command } > #{ options[:output] }"
  end

  `#{ command }`

  raise "Filed to execute '#{ command }'" if $?.exitstatus != 0
end

def read_param filename, param
  File.open(filename) do |file|
    while line = file.gets
      return $1 if line =~ /^#{ param }\s+(.*)$/
    end
  end
  nil
end
