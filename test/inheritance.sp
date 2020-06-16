struct super_struct
  fn act()
    println('super_struct acting')
  end
end

module feature0
  fn jump()
    println('jumping')
  end
end

module feature1
  fn run()
    println('running')
  end
end

struct sub_struct < super_struct
  include feature0
  include feature1
end

struct sub_struct2 < super_struct
  fn act()
    println('sub_struct2 acting')
  end
end

sub_struct.act()
sub_struct.jump()
sub_struct.run()
sub_struct2.act()