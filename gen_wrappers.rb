#!/usr/bin/env ruby -w

JUST_WRAP = %w{glutDisplayFunc glutKeyboardFunc glutInitDisplayMode glutSolidSphere glutAddMenuEntry glutAddSubMenu
    glutCreateMenu glutChangeToMenuEntry glutAttachMenu glutSolidIcosahedron glutSolidTetrahedron glutSolidTorus
    glutVisibilityFunc glutMotionFunc glutMouseFunc glutSolidDodecahedron glutSolidTeapot glutStrokeCharacter
    glutStrokeRoman glutSolidCube glutWireCube glutSetWindowTitle glutSpecialFunc}
EXTRA_SYMS = %w{glutStrokeRoman}

fail "usage: #{$0} <prefix> <path>" unless ARGV.length == 2
prefix = ARGV[0]
path = ARGV[1]

funcs = []
while $stdin.gets
  next unless /^extern (.*) APIENTRY (\w+)\((.+)\) /
  type, name, args = $1, $2, $3
  wrapper_name = name.sub(/^#{prefix}/, "open#{prefix.upcase}")
  args.gsub!(/\(\*\)/, '(*func)')
  arg_names = args.gsub(/\(\*(\w*)\)\(.*\)/, '\\1')
  if $-d && arg_names != args
    $stderr.puts "#{args} => #{arg_names}"
  end
  arg_names = arg_names.split(/, /).map do |tokens|
    arg_name = tokens.split(/[ *]/).last
    arg_name = 'func' if arg_name == ''
    arg_name
  end.select do |arg_name|
    arg_name != 'void'
  end.join(', ')
  funcs << [type, name, args, wrapper_name, arg_names]
end

funcs.each do |func|
  puts "static #{func[0]} (*#{func[3]})(#{func[2]}) = NULL;"
end

def gen_resolve(public_sym, private_sym)
  puts <<EOT
\tif (!(#{public_sym} = dlsym(handle, \"#{private_sym}\")))
\t\terr(1, \"Could not resolve #{private_sym}()\");
EOT
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
  gen_resolve(func[3], func[1])
end
EXTRA_SYMS.each do |sym|
  gen_resolve(sym, sym)
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

