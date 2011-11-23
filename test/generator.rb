require File.join File.dirname(__FILE__), 'common'

class Generator

  def self.tgffopt options
    gaps = {
      :task_count => options[:task_count],
      :type_count => options[:task_count] / 2,
      :processor_count => options[:processor_count],
      :seed => options[:seed]
    }

    File.open(options[:output], 'w') do |file|
      options[:template].complete(gaps) do |line|
        file << line
      end
    end
  end

  def self.tgff options
    run \
      :tool => [ 'vendor', 'tgff' ],
      :arguments => options[:input].gsub(/\.tgffopt$/, '')
  end

  def self.validate_tgff options
    task_count = 0;

    File.open(options[:input]) do |file|
      while line = file.gets
        task_count += 1 if line =~ /\s*TASK\s+/
      end
    end

    task_count == options[:task_count]
  end

  def self.system options
    run \
      :tool => [ 'tools', 'system' ],
      :arguments => options[:input],
      :output => options[:output]
  end

  def self.params options
    population_size = 4 * options[:task_count]

    gaps = {
      :repeat => options[:repeat_count],
      :population_size => population_size,
      :stall_generations => STALL_GENERATIONS,
      :tournament_size => 2
    }

    File.open(options[:output], 'w') do |file|
      options[:template].complete(gaps) do |line|
        file << line
      end
    end
  end

  def self.floorplan options
    run \
      :tool => [ 'tools', 'floorplan' ],
      :arguments => [ options[:processor_count], DIE_AREA ],
      :output => options[:output]
  end

  def self.hotspot options
    File.open(options[:output], 'w') do |file|
      options[:template].complete({}) do |line|
        file << line
      end
    end
  end
end
