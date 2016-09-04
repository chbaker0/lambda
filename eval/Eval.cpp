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
        node->argumentTag = term.arguments[0];

        ast::Abstraction *last = node.get();

        for (unsigned int argNdx = 1; argNdx < term.arguments.size(); ++argNdx)
        {
            std::unique_ptr<ast::Abstraction> current(new ast::Abstraction);
            current->argumentTag = term.arguments[argNdx];

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

} // anonymous

std::unique_ptr<ast::Term> convertParseTree(const parse::tree::Term& parseTree)
{
    ParseTreeConverter parseTreeConverter;
    parseTree.applyVisitor(parseTreeConverter);
    return std::move(parseTreeConverter.result);
}

} // eval
