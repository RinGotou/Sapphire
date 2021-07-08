fn Generator(value)
  i = value
  fn Handler()
    println(i)
    i += 1
  end
  
  return Handler
end

func0 = Generator(1)
func0()
func0()
func0()