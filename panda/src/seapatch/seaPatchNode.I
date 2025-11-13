INLINE bool SeaPatchNode::is_enabled() const {
  return _enabled;
}

INLINE void SeaPatchNode::enable(){
  _enabled=true;
}

INLINE void SeaPatchNode::disable(){
  _enabled=false;
}

INLINE void SeaPatchNode::set_want_reflect(bool flag) {
  _want_reflect = flag;
}

INLINE bool SeaPatchNode::get_want_reflect() const {
  return _want_reflect;
}

INLINE void SeaPatchNode::set_want_color(bool flag) {
  _want_color = flag;
}

INLINE bool SeaPatchNode::get_want_color() const {
  return _want_color;
}