#!/usr/bin/env ruby -w

funcs = []
while $stdin.gets
  if /^\t(.*) \(\*(.+)\)\((.*)\)/
    funcs << [$1, $2, $3]
  end
end

File.open('gl_setup_ctx.c', 'w') do |f|
  funcs.each do |func|
    f.puts "\tcontext->disp.#{func[1]} = gl_#{func[1]};"
  end
end

no_wrap = []
File.open('gl.c', 'r') do |f|
  while f.gets
    if /^gl_([^(]+)\(/
      no_wrap << $1
    end
  end
end

File.open('gl_stubs.c', 'w') do |f|
  funcs.each do |func|
    next if no_wrap.include?(func[1])
    ret = case func[0]
          when 'void' then ''
          when 'GLboolean' then "\n\treturn GL_FALSE;"
          when 'GLuint' then "\n\treturn 0;"
          when 'GLenum' then "\n\treturn GL_INVALID_ENUM;"
          when 'const GLubyte*' then "\n\treturn NULL;"
          when 'GLintptr', 'GLint' then "\n\treturn -1;"
          else fail "Don't know how to return #{func[0]}"
          end
    f.puts %Q{\nstatic #{func[0]}\ngl_#{func[1]}(#{func[2]})\n{\n\tfprintf(stderr, "TODO: #{func[1]}()\\n");#{ret}\n}\n}
  end
end

