#include "Eval.hpp"

#include <deque>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

namespace eval
{

namespace ast
{

void Abstraction::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

void Application::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

void BoundVariable::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

void FreeVariable::applyVisitor(TermVisitor& termVisitor) const
{
    termVisitor.acceptTerm(*this);
}

} // ast

namespace
{

using namespace parse;

class ParseTreeConverter : public parse::tree::TermVisitor
{
public:

    virtual void acceptTerm(const tree::Abstraction& term) override
    {
        std::unique_ptr<ast::Abstraction> node(new ast::Abstraction);
        node->argumentName = term.arguments[0];

        ast::Abstraction *last = node.get();

        for (unsigned int argNdx = 1; argNdx < term.arguments.size(); ++argNdx)
        {
            std::unique_ptr<ast::Abstraction> current(new ast::Abstraction);
            current->argumentName = term.arguments[argNdx];

            ast::Abstraction *temp = last;

            last = current.get();
            temp->body = std::move(current);
        }

        ParseTreeConverter bodyConverter;
        term.body->applyVisitor(bodyConverter);
        last->body = std::move(bodyConverter.result);

        result = std::move(node);
    }

    virtual void acceptTerm(const tree::Application& term) override
    {
        if (term.terms.size() < 2)
        {
            throw std::runtime_error("Invalid parse tree provided as input");
        }

        std::deque<std::unique_ptr<ast::Term>> convertedTerms;

        for (const std::unique_ptr<parse::tree::Term>& cur : term.terms)
        {
            ParseTreeConverter termConverter;
            cur->applyVisitor(termConverter);
            convertedTerms.push_back(std::move(termConverter.result));
        }

        std::unique_ptr<ast::Application> node(new ast::Application);
        node->left = std::move(convertedTerms[0]);
        node->right = std::move(convertedTerms[1]);

        convertedTerms.pop_front();
        convertedTerms.pop_front();

        while (!convertedTerms.empty())
        {
            std::unique_ptr<ast::Application> newNode(new ast::Application);
            newNode->left = std::move(node);
            newNode->right = std::move(convertedTerms.front());

            node = std::move(newNode);
            convertedTerms.pop_front();
        }

        result = std::move(node);
    }

    virtual void acceptTerm(const tree::Variable& term) override
    {
        std::unique_ptr<ast::FreeVariable> node(new ast::FreeVariable);
        node->name = term.name;
        result = std::move(node);
    }

    std::unique_ptr<ast::Term> result;
};

class NameResolver : public ast::TermVisitor
{
public:

    NameResolver(std::map<std::string, unsigned int>& symbolTable_in, unsigned int depth_in = 0)
        : symbolTable(symbolTable_in), depth(depth_in) {}

    virtual void acceptTerm(const ast::Abstraction& term) override
    {
        std::string argName = term.argumentName;

        bool saved = false;
        unsigned int savedIndex = 0;

        auto it = symbolTable.find(argName);
        if (it != symbolTable.end())
        {
            saved = true;
            savedIndex = it->second;
            it->second = depth;
        }
        else
        {
            symbolTable[argName] = depth;
        }

        NameResolver bodyVisitor(symbolTable, depth + 1);
        term.body->applyVisitor(bodyVisitor);

        std::unique_ptr<ast::Abstraction> resolvedTerm(new ast::Abstraction);
        resolvedTerm->argumentName = argName;
        resolvedTerm->body = std::move(bodyVisitor.result);
        result = std::move(resolvedTerm);

        if (saved)
        {
            symbolTable[argName] = savedIndex;
        }
        else
        {
            symbolTable.erase(argName);
        }
    }

    virtual void acceptTerm(const ast::Application& term) override
    {
        std::unique_ptr<ast::Application> resolvedTerm(new ast::Application);

        NameResolver visitor(symbolTable, depth);
        term.left->applyVisitor(visitor);
        resolvedTerm->left = std::move(visitor.result);
        term.right->applyVisitor(visitor);
        resolvedTerm->right = std::move(visitor.result);

        result = std::move(resolvedTerm);
    }

    virtual void acceptTerm(const ast::FreeVariable& term) override
    {
        auto it = symbolTable.find(term.name);
        if (it != symbolTable.end())
        {
            std::unique_ptr<ast::BoundVariable> resolvedTerm(new ast::BoundVariable);
            resolvedTerm->index = depth - it->second;
            result = std::move(resolvedTerm);
        }
        else
        {
            std::unique_ptr<ast::FreeVariable> unresolvedTerm(new ast::FreeVariable);
            unresolvedTerm->name = term.name;
            result = std::move(unresolvedTerm);
        }
    }

    virtual void acceptTerm(const ast::BoundVariable& term) override
    {
        std::unique_ptr<ast::BoundVariable> duplicate(new ast::BoundVariable);
        duplicate->index = term.index;
        result = std::move(duplicate);
    }

    std::map<std::string, unsigned int>& symbolTable;
    unsigned int depth;
    std::unique_ptr<ast::Term> result;
};

} // anonymous

std::unique_ptr<ast::Term> convertParseTree(const parse::tree::Term& parseTree)
{
    ParseTreeConverter parseTreeConverter;
    parseTree.applyVisitor(parseTreeConverter);

    std::unique_ptr<ast::Term> unresolvedTree = std::move(parseTreeConverter.result);

    std::map<std::string, unsigned int> symbolTable;
    NameResolver nameResolver(symbolTable);
    unresolvedTree->applyVisitor(nameResolver);

    return std::move(nameResolver.result);
}

} // eval
