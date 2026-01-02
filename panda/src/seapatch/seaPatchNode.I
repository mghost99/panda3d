INLINE SeaPatchNode::SeaPatchNode(const std::string& name, PT(SeaPatchRoot) root) : PandaNode(name),
  m_root(root),
  m_want_z(true),
  m_want_reflect(true),
  m_want_color(true),
  m_want_uv(true),
  m_want_normal(true),
  m_z_offset(0.0),
  m_last_update(-1.0f)
{
    set_cull_callback();
}

INLINE SeaPatchNode::~SeaPatchNode()
{
    m_root = NULL;
}

INLINE void SeaPatchNode::set_want_z(bool want_z)
{
    m_want_z = want_z;
}

INLINE bool SeaPatchNode::get_want_z()
{
    return m_want_z;
}

INLINE void SeaPatchNode::set_want_reflect(bool want_reflect)
{
    m_want_reflect = want_reflect;
}

INLINE bool SeaPatchNode::get_want_reflect()
{
    return m_want_reflect;
}

INLINE void SeaPatchNode::set_want_color(bool want_color)
{
    m_want_color = want_color;
}

INLINE bool SeaPatchNode::get_want_color()
{
    return m_want_color;
}

INLINE void SeaPatchNode::set_want_uv(bool want_uv)
{
    m_want_uv = want_uv;
}

INLINE bool SeaPatchNode::get_want_uv()
{
    return m_want_uv;
}

INLINE void SeaPatchNode::set_want_normal(bool want_normal)
{
    m_want_normal = want_normal;
}

INLINE bool SeaPatchNode::get_want_normal()
{
    return m_want_normal;
}

INLINE void SeaPatchNode::set_z_offset(float offset)
{
    m_z_offset = offset;
}

INLINE float SeaPatchNode::get_z_offset()
{
    return m_z_offset;
}

INLINE void SeaPatchNode::calc_reflection(LVecBase3f& normal, LPoint3f& d, LVecBase3f& result)
{
    float v12 = normal.dot(d);
    LPoint3f v3;
    LVecBase3f v4;
    v3 = normal * 2 * v12;
    v4 = d - v3;
    v4.normalize();
    result[0] = v4[0];
    result[1] = v4[1];
    result[2] = v4[2];
}
