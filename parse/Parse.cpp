#include "Parse.hpp"

#include <iostream>
#include <stdexcept>
#include <cctype>

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

namespace
{

void skipWhitespace(std::istream& input)
{
    char c = input.peek();

    while (std::isspace(c))
    {
        input.get();
        c = input.peek();
    }
}

std::string parseIdentifier(std::istream& input)
{
    std::string ident;

    skipWhitespace(input);

    while (std::isalnum(input.peek()))
    {
        ident += input.get();
    }

    return ident;
}

std::unique_ptr<tree::Abstraction> parseAbstraction(std::istream& input)
{
    std::unique_ptr<tree::Abstraction> node(new tree::Abstraction);

    if (input.peek() != '^')
    {
        throw std::runtime_error("Parse error");
    }

    input.get();

    std::string argument;

    while ((argument = parseIdentifier(input)).size() > 0)
    {
        node->arguments.push_back(std::move(argument));
    }

    if (node->arguments.size() == 0)
    {
        throw std::runtime_error("Parse error");
    }

    skipWhitespace(input);

    if (input.peek() != '.')
    {
        throw std::runtime_error("Parse error");
    }

    input.get();

    node->body = parseTerm(input);

    return node;
}

std::unique_ptr<tree::Term> parseSubterm(std::istream& input)
{
    std::unique_ptr<tree::Term> node;

    skipWhitespace(input);

    if (input.peek() == '^')
    {
        node = parseAbstraction(input);
    }
    else if (input.peek() == '(')
    {
        input.get();

        node = parseTerm(input);

        skipWhitespace(input);

        if (input.peek() != ')')
        {
            throw std::runtime_error("Parse error");
        }

        input.get();
    }
    else
    {
        std::string name = parseIdentifier(input);

        if (name.size() == 0)
        {
            throw std::runtime_error("Parse error");
        }

        std::unique_ptr<tree::Variable> var(new tree::Variable);
        var->name = std::move(name);

        node = std::move(var);
    }

    return node;
}

} // anonymous

std::unique_ptr<tree::Term> parseTerm(std::istream& input)
{
    std::unique_ptr<tree::Term> node = parseSubterm(input);

    if (input.peek() != EOF)
    {
        std::unique_ptr<tree::Term> tempNode = std::move(node);
        std::unique_ptr<tree::Application> appNode = std::unique_ptr<tree::Application>(new tree::Application);
        appNode->terms.push_back(std::move(tempNode));

        do
        {
            appNode->terms.push_back(parseSubterm(input));
        } while (input.peek () != EOF);

        node = std::move(appNode);
    }

    return node;
}

} // parse
