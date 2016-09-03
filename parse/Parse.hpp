#include <memory>
#include <string>
#include <vector>

namespace parse
{

namespace tree
{

class TermVisitor;

class Term
{
public:

    virtual void applyVisitor(TermVisitor&) const = 0;
};

class Abstraction : public Term
{
public:

    std::vector<std::string> arguments;
    std::unique_ptr<Term> body;

    virtual void applyVisitor(TermVisitor&) const override;
};

class Application : public Term
{
public:

    std::vector<std::unique_ptr<Term>> terms;

    virtual void applyVisitor(TermVisitor&) const override;
};

class Variable : public Term
{
public:

    std::string name;

    virtual void applyVisitor(TermVisitor&) const override;
};

class TermVisitor
{
public:

    virtual void acceptTerm(const Abstraction&) = 0;
    virtual void acceptTerm(const Application&) = 0;
    virtual void acceptTerm(const Variable&) = 0;
};

} // tree

} // parse
