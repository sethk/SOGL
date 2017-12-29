#!/usr/bin/env ruby -w

JUST_WRAP = %w{glutDisplayFunc glutKeyboardFunc glutGetWindow glutInitDisplayMode glutInitWindowSize glutMainLoop}

fail "usage: #{$0} <prefix> <path>" unless ARGV.length == 2
prefix = ARGV[0]
path = ARGV[1]

funcs = []
while $stdin.gets
  next unless /^extern (.*) APIENTRY (\w+)\((.+)\) /
  type, name, args = $1, $2, $3
  wrapper_name = name.sub(/^#{prefix}/, "open#{prefix.upcase}")
  arg_names = args.gsub(/\(\*(\w+)\)\(.*\)/, '\\1')
  if $-d && arg_names != args
    $stderr.puts "#{args} => #{arg_names}"
  end
  arg_names = arg_names.split(/, /).map do |tokens|
    tokens.split.last
  end.select do |arg_name|
    arg_name != 'void'
  end.join(', ')
  funcs << [type, name, args, wrapper_name, arg_names]
end

funcs.each do |func|
  puts "static #{func[0]} (*#{func[3]})(#{func[2]}) = NULL;"
end

puts <<EOT
static void
open#{prefix}_init(void)
{
\tvoid *handle;
\tif (!(handle = dlopen(\"#{path}\", RTLD_LAZY | RTLD_LOCAL)))
\t\terr(1, "Could not dlopen #{path}");
EOT
funcs.each do |func|
  puts <<EOT
\tif (!(#{func[3]} = dlsym(handle, \"#{func[1]}\")))
\t\terr(1, \"Could not resolve #{func[1]}()\");
EOT
end
puts "}"

funcs.each do |func|
  if JUST_WRAP.include?(func[1])
    ret = (func[0] != 'void') ? 'return ' : ''
    puts <<EOT
#{func[0]}
#{func[1]}(#{func[2]})
{
\t#{ret}#{func[3]}(#{func[4]});
}
EOT
  end
end

