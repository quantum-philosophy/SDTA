class Template
  def initialize filename
    @template = []

    File.open(filename) do |file|
      while line = file.gets
        next if line =~ /\s*#/ or line =~ /^\s*$/
        @template << line
      end
    end
  end

  def complete options, &block
    gaps = {}

    options.each do |param, value|
      value = value.to_s

      if value.empty?
        puts "Got a blank value for '#{ param }'"
        exit
      end

      gaps["[#{ param.to_s.upcase }]"] = value
    end

    @template.each do |line|
      gaps.each do |param, value|
        line = line.gsub param, value
      end
      if line =~ /\[([A-Z_]+)\]/
        puts "Do not know how to fill in '#{ $1.downcase }'"
        exit
      end
      yield line
    end
  end
end
