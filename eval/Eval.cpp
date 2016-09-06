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

std::unique_ptr<Term> Abstraction::copy() const
{
    std::unique_ptr<Abstraction> result(new Abstraction);
    result->argumentName = argumentName;
    result->body = body->copy();
    return result;
}

std::unique_ptr<Term> Application::copy() const
{
    std::unique_ptr<Application> result(new Application);
    result->left = left->copy();
    result->right = right->copy();
    return result;
}

std::unique_ptr<Term> BoundVariable::copy() const
{
    std::unique_ptr<BoundVariable> result(new BoundVariable);
    result->index = index;
    return result;
}

std::unique_ptr<Term> FreeVariable::copy() const
{
    std::unique_ptr<FreeVariable> result(new FreeVariable);
    result->name = name;
    return result;
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

class BoundVariableReplacer : public ast::TermVisitor
{
public:

    BoundVariableReplacer(unsigned int boundIndex_in, const ast::Term *prototype_in)
        : boundIndex(boundIndex_in), prototype(prototype_in) {}

    virtual void acceptTerm(const ast::Abstraction& term) override
    {
        BoundVariableReplacer bodyReplacer(boundIndex + 1, prototype);
        term.body->applyVisitor(bodyReplacer);

        std::unique_ptr<ast::Abstraction> replacedTerm(new ast::Abstraction);
        replacedTerm->argumentName = term.argumentName;
        replacedTerm->body = std::move(bodyReplacer.result);

        result = std::move(replacedTerm);
    }

    virtual void acceptTerm(const ast::Application& term) override
    {
        std::unique_ptr<ast::Application> replacedTerm(new ast::Application);

        BoundVariableReplacer subtermReplacer(boundIndex, prototype);
        term.left->applyVisitor(subtermReplacer);
        replacedTerm->left = std::move(subtermReplacer.result);
        term.right->applyVisitor(subtermReplacer);
        replacedTerm->right = std::move(subtermReplacer.result);

        result = std::move(replacedTerm);
    }

    virtual void acceptTerm(const ast::BoundVariable& term) override
    {
        if (term.index == boundIndex)
        {
            result = prototype->copy();
        }
        else
        {
            result = term.copy();
        }
    }

    virtual void acceptTerm(const ast::FreeVariable& term) override
    {
        result = term.copy();
    }

    unsigned int boundIndex;
    const ast::Term *prototype;

    std::unique_ptr<ast::Term> result;
};

class BetaReducer : public ast::TermVisitor
{
private:

    enum TermType
    {
        Abstraction,
        Application,
        FreeVariable,
        BoundVariable,
        Invalid
    };

    class TermIdentifier : public ast::TermVisitor
    {
    public:

        TermIdentifier() : result(Invalid) {}

        virtual void acceptTerm(const ast::Abstraction&) override
        {
            result = Abstraction;
        }

        virtual void acceptTerm(const ast::Application&) override
        {
            result = Application;
        }

        virtual void acceptTerm(const ast::FreeVariable&) override
        {
            result = FreeVariable;
        }

        virtual void acceptTerm(const ast::BoundVariable&) override
        {
            result = BoundVariable;
        }

        TermType result;
    };

public:

    BetaReducer() : reduced(false) {}

    virtual void acceptTerm(const ast::Application& term) override
    {
        TermIdentifier identifier;
        term.left->applyVisitor(identifier);

        TermType lhsIdent = identifier.result;
        if (lhsIdent == Abstraction)
        {
            BoundVariableReplacer replacer(1, term.right.get());
            static_cast<const ast::Abstraction*>(term.left.get())->body->applyVisitor(replacer);
            result = std::move(replacer.result);
            reduced = true;
        }
        else
        {
            std::unique_ptr<ast::Application> reducedTerm(new ast::Application);

            BetaReducer subtermReducer;
            term.left->applyVisitor(subtermReducer);
            reducedTerm->left = std::move(subtermReducer.result);
            reduced = reduced || subtermReducer.reduced;

            term.right->applyVisitor(subtermReducer);
            reducedTerm->right = std::move(subtermReducer.result);
            reduced = reduced || subtermReducer.reduced;

            result = std::move(reducedTerm);
        }
    }

    virtual void acceptTerm(const ast::Abstraction& term) override
    {
        BetaReducer bodyReducer;
        term.body->applyVisitor(bodyReducer);
        reduced = bodyReducer.reduced;

        std::unique_ptr<ast::Abstraction> reducedTerm(new ast::Abstraction);
        reducedTerm->argumentName = term.argumentName;
        reducedTerm->body = std::move(bodyReducer.result);

        result = std::move(reducedTerm);
    }

    virtual void acceptTerm(const ast::BoundVariable& term) override
    {
        reduced = false;
        result = term.copy();
    }

    virtual void acceptTerm(const ast::FreeVariable& term) override
    {
        reduced = false;
        result = term.copy();
    }

    bool reduced;
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

std::unique_ptr<ast::Term> betaReduce(const ast::Term& tree)
{
    BetaReducer betaReducer;
    tree.applyVisitor(betaReducer);

    if (betaReducer.reduced)
    {
        return std::move(betaReducer.result);
    }
    else
    {
        return std::unique_ptr<ast::Term>();
    }
}

} // eval
