chdir()

sample_extension = extension('extension_sample.dll')
if sample_extension.good()
  sample_helloworld = sample_extension.fetch('sample_helloworld')
  sample_plus = sample_extension.fetch('sample_plus')
  sample_throw_error = sample_extension.fetch('sample_throw_error')
  sample_variable_print = sample_extension.fetch('sample_variable_print')
  
  println(sample_helloworld())
  println(sample_plus(1, 2))
  println(sample_variable_print(1,2,3,4,5,6,7))
  sample_throw_error()
end