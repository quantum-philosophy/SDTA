require File.join File.dirname(__FILE__), 'common'

class Generator

  def self.tgffopt options
    gaps = {
      :task_count => options[:task_count],
      :type_count => options[:task_count] / 2,
      :processor_count => options[:processor_count]
    }

    File.open(options[:output], 'w') do |file|
      gaps[:seed] = rand 1e5

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
    tournament_size = [ 2, POPULATION_FACTOR * options[:task_count] / 100 + 1 ].max
    population_size = tournament_size * 100

    gaps = {
      :repeat => GA_REALIZATIONS_PER_GRAPH,
      :population_size => population_size,
      :stall_generations => STALL_GENERATIONS,
      :tournament_size => tournament_size
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
