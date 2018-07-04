#if !defined(FOR_EACH_UNIFORM_TYPE)
#  error "Define FOR_EACH_UNIFORM_TYPE before including ForEachUniformType.inl"
#endif

// uniform_type, data_type, param_type

FOR_EACH_UNIFORM_TYPE(Float, float, float)
FOR_EACH_UNIFORM_TYPE(Int, int, int)
FOR_EACH_UNIFORM_TYPE(Uint, unsigned int, unsigned int)
#if !defined(FOR_EACH_UNIFORM_TYPE_NO_BOOL)
FOR_EACH_UNIFORM_TYPE(Bool, bool, bool)
#endif

FOR_EACH_UNIFORM_TYPE(Float2, glm::fvec2, const glm::fvec2&)
FOR_EACH_UNIFORM_TYPE(Float3, glm::fvec3, const glm::fvec3&)
FOR_EACH_UNIFORM_TYPE(Float4, glm::fvec4, const glm::fvec4&)

FOR_EACH_UNIFORM_TYPE(Int2, glm::ivec2, const glm::ivec2&)
FOR_EACH_UNIFORM_TYPE(Int3, glm::ivec3, const glm::ivec3&)
FOR_EACH_UNIFORM_TYPE(Int4, glm::ivec4, const glm::ivec4&)

FOR_EACH_UNIFORM_TYPE(Uint2, glm::uvec2, const glm::uvec2&)
FOR_EACH_UNIFORM_TYPE(Uint3, glm::uvec3, const glm::uvec3&)
FOR_EACH_UNIFORM_TYPE(Uint4, glm::uvec4, const glm::uvec4&)

#if !defined(FOR_EACH_UNIFORM_TYPE_NO_BOOL)
FOR_EACH_UNIFORM_TYPE(Bool2, glm::bvec2, const glm::bvec2&)
FOR_EACH_UNIFORM_TYPE(Bool3, glm::bvec3, const glm::bvec3&)
FOR_EACH_UNIFORM_TYPE(Bool4, glm::bvec4, const glm::bvec4&)
#endif

FOR_EACH_UNIFORM_TYPE(Float2x2, glm::fmat2x2, const glm::fmat2x2&)
FOR_EACH_UNIFORM_TYPE(Float2x3, glm::fmat2x3, const glm::fmat2x3&)
FOR_EACH_UNIFORM_TYPE(Float2x4, glm::fmat2x4, const glm::fmat2x4&)
FOR_EACH_UNIFORM_TYPE(Float3x2, glm::fmat3x2, const glm::fmat3x2&)
FOR_EACH_UNIFORM_TYPE(Float3x3, glm::fmat3x3, const glm::fmat3x3&)
FOR_EACH_UNIFORM_TYPE(Float3x4, glm::fmat3x4, const glm::fmat3x4&)
FOR_EACH_UNIFORM_TYPE(Float4x2, glm::fmat4x2, const glm::fmat4x2&)
FOR_EACH_UNIFORM_TYPE(Float4x3, glm::fmat4x3, const glm::fmat4x3&)
FOR_EACH_UNIFORM_TYPE(Float4x4, glm::fmat4x4, const glm::fmat4x4&)

#if !defined(FOR_EACH_UNIFORM_TYPE_NO_COMPLEX)
FOR_EACH_UNIFORM_TYPE(Texture, SPtr<Texture>, const SPtr<Texture>&)
#endif
