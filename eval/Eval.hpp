#ifndef EVAL_HPP_INCLUDED
#define EVAL_HPP_INCLUDED

#include <memory>
#include <string>

#include "Parse.hpp"

namespace eval
{

namespace ast
{

class TermVisitor;

class Term
{
public:

    virtual ~Term() {}

    virtual void applyVisitor(TermVisitor&) const = 0;
    virtual std::unique_ptr<Term> copy() const = 0;
};

class Abstraction : public Term
{
public:

    std::string argumentName;
    std::unique_ptr<Term> body;

    virtual void applyVisitor(TermVisitor&) const override;
    virtual std::unique_ptr<Term> copy() const override;
};

class Application : public Term
{
public:

    std::unique_ptr<Term> left, right;

    virtual void applyVisitor(TermVisitor&) const override;
    virtual std::unique_ptr<Term> copy() const override;
};

class BoundVariable : public Term
{
public:

    unsigned int index;

    virtual void applyVisitor(TermVisitor&) const override;
    virtual std::unique_ptr<Term> copy() const override;
};

class FreeVariable : public Term
{
public:

    std::string name;

    virtual void applyVisitor(TermVisitor&) const override;
    virtual std::unique_ptr<Term> copy() const override;
};

class TermVisitor
{
public:

    virtual void acceptTerm(const Abstraction&) = 0;
    virtual void acceptTerm(const Application&) = 0;
    virtual void acceptTerm(const BoundVariable&) = 0;
    virtual void acceptTerm(const FreeVariable&) = 0;
};

} // ast

std::unique_ptr<ast::Term> convertParseTree(const parse::tree::Term&);
std::unique_ptr<ast::Term> betaReduce(const ast::Term&);

} // eval

#endif // EVAL_HPP_INCLUDED
