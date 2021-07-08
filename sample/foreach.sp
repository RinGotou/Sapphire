arr = {1, 2, 3, 4, 5, 6, 7}

for unit in arr
  println(unit)
end

tbl = table()
tbl.insert('key0', 'value0')
tbl.insert('key1', 'value1')
tbl.insert('key2', 'value2')

for unit in tbl
  println('key=' + unit.left() + ' value=' + unit.right())
end

struct iterator
  attribute index
  
  fn initializer(index)
    me.index = index
  end
  
  fn get()
    return me.index
  end
  
  fn step()
    me.index += 1
	println('index' + me.index)
  end
  
  fn compare(rhs)
    return rhs.index == me.index
  end
end

struct base
  fn head()
    return iterator(0)
  end
  
  fn tail()
    return iterator(3)
  end
  
  fn empty()
    return false
  end
end

for unit in base
  println(unit)
end