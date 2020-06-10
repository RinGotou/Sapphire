=begin
  Type facilities for Kagami Scripting Language
  Copyright(c) 2020, Rin Gotou
  Licensed under BSD-2-Clause
=end

fn is_plain_type(obj)
  local type_id = typeid(obj)
  local result = false
  
  case type_id
  when kTypeIdInt, kTypeIdFloat, kTypeIdString, kTypeIdBool
    result = true
  end
  
  return result
end

fn is_calculatable(obj)
  local type_id = typeid(obj)
  local result = false
  
  case type_id
  when kTypeIdInt, kTypeIdFloat
    result = true
  end
  
  if !result
    result = has_behavior(obj, 'to_int|to_float')
  end
  
  return result
end

fn is_comparable(obj)
  return exist(obj, '__compare')
end

fn is_printable(obj)
  return exist(obj, 'print')
end