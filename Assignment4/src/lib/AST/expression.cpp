#include "AST/expression.hpp"

void ExpressionNode::setType(std::string type)
{
    if (type == "integer")
        m_type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kIntegerType);
    else if (type == "real")
        m_type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kRealType);
    else if (type == "boolean")
        m_type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kBoolType);
    else if (type == "string")
        m_type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kStringType);
    else if (type == "void")
        m_type = std::make_shared<PType>(PType::PrimitiveTypeEnum::kVoidType);
    else
        m_type = nullptr;
}