#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "parse/Parse.hpp"
#include "eval/Eval.hpp"

using namespace parse;
using namespace eval;
using std::cout;

class TreePrinter : public tree::TermVisitor
{
public:

    virtual void acceptTerm(const tree::Abstraction& term) override
    {
        cout << "(^";

        for (const std::string& arg : term.arguments)
        {
            cout << arg << " ";
        }

        cout << ". ";

        term.body->applyVisitor(*this);

        cout << ")";
    }

    virtual void acceptTerm(const tree::Application& term) override
    {
        cout << "(";

        bool isFirst = true;

        for (unsigned int termNdx = 0; termNdx < term.terms.size(); ++termNdx)
        {
            if (!isFirst)
            {
                cout << " ";
            }

            isFirst = false;

            term.terms[termNdx]->applyVisitor(*this);
        }

        cout << ")";
    }

    virtual void acceptTerm(const tree::Variable& term) override
    {
        cout << term.name;
    }
};

class ASTPrinter : public ast::TermVisitor
{
public:

    virtual void acceptTerm(const ast::Abstraction& term) override
    {
        cout << "(^" << term.argumentName << ". ";
        term.body->applyVisitor(*this);
        cout << ")";
    }

    virtual void acceptTerm(const ast::Application& term) override
    {
        cout << "(";
        term.left->applyVisitor(*this);
        cout << " ";
        term.right->applyVisitor(*this);
        cout << ")";
    }

    virtual void acceptTerm(const ast::BoundVariable& term) override
    {
        cout << "_" << term.index;
    }

    virtual void acceptTerm(const ast::FreeVariable& term) override
    {
        cout << term.name;
    }
};

int main()
{
    std::string input;

    cout << ">> ";

    while (getline(std::cin, input) && input.size() > 0)
    {
        std::istringstream is(input);

        try
        {
            std::unique_ptr<tree::Term> tree = parseTerm(is);

            TreePrinter treePrinter;
            tree->applyVisitor(treePrinter);

            cout << "\n\n";

            std::unique_ptr<ast::Term> ast = convertParseTree(*tree);

            bool isDone = false;
            while (!isDone)
            {
                ASTPrinter astPrinter;
                ast->applyVisitor(astPrinter);

                cout << "\n\n";

                std::unique_ptr<ast::Term> reduced = betaReduce(*ast);
                if (reduced)
                {
                    ast = std::move(reduced);
                }
                else
                {
                    isDone = true;
                }
            }
        }
        catch (std::runtime_error& e)
        {
            cout << e.what() << "\n";
        }

        cout << ">> ";
    }
}
