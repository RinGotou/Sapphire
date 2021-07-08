println(filesystem.core())
println(filesystem.boot())
println(filesystem.current())

println(filesystem.exist('C:\\sp_workbench\\main.sp'))
println(filesystem.exist('C:\\ntldr'))
println(filesystem.extension('C:\\sp_workbench\\main.sp'))

struct worker < filesystem
  fn test(); end
end

println(worker.core())
println(exist(worker, 'test'))