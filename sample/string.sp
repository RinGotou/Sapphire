str0 = 'hello\nworld\nby\nsapphire'
str1 = '   everyone    '
wstr0 = ' 吾輩は猫です '

arr0 = str0.split_by('\n')
str3 = str1.trim()
wstr1 = wstr0.trim()

for unit in arr0
  println(unit)
end

println(str3)
println(wstr1)
