#include "Parse.hpp"

namespace parse
{

namespace tree
{

void Abstraction::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

void Application::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

void Variable::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

} // tree

} // parse
