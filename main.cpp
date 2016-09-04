#include <iostream>
#include <memory>
#include <sstream>
#include <string>
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
    std::string input;

    while (getline(std::cin, input) && input.size() > 0)
    {
        std::istringstream is(input);

        try
        {
            std::unique_ptr<tree::Term> tree = parseTerm(is);

            TreePrinter treePrinter;
            tree->applyVisitor(treePrinter);

            cout << "\n";
        }
        catch (std::runtime_error& e)
        {
            cout << e.what() << "\n";
        }
    }
}
