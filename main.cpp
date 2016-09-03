#include <iostream>
#include <memory>
#include <utility>

#include "parse/Parse.hpp"

using namespace parse;
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

        for (int termNdx = 0; termNdx < term.terms.size(); ++termNdx)
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

int main()
{
    std::unique_ptr<tree::Variable> vx(new tree::Variable), vy(new tree::Variable);

    vx->name = "x";
    vy->name = "y";

    std::unique_ptr<tree::Application> body(new tree::Application);

    body->terms.push_back(std::move(vx));
    body->terms.push_back(std::move(vy));

    std::unique_ptr<tree::Abstraction> abs(new tree::Abstraction);

    abs->arguments.push_back("x");
    abs->body = std::move(body);

    TreePrinter treePrinter;

    abs->applyVisitor(treePrinter);

    cout << "\n";
}
