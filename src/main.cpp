#include <algorithm>
#include <cctype>
#include <cmath>
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui.h"
#include <GLFW/glfw3.h>

enum class TokenType
{
    Number,
    Operator,
    Function,
    LeftParen,
    RightParen
};

struct Token
{
    TokenType   type;
    std::string value;
};

class Calculator
{
public:
    double evaluate(const std::string& expression)
    {
        auto tokens = tokenize(expression);
        auto rpn    = shuntingYard(tokens);
        return solveRPN(rpn);
    }

private:
    int getPrecedence(const std::string& op)
    {
        if (op == "+" || op == "-")
            return 1;
        if (op == "*" || op == "/" || op == "%")
            return 2;
        if (op == "^")
            return 3;
        return 0;
    }

    bool isRightAssociative(const std::string& op) { return op == "^"; }

    std::vector< Token > tokenize(const std::string& expr)
    {
        std::vector< Token > tokens;
        for (size_t i = 0; i < expr.length(); ++i)
        {
            char c = expr[i];
            if (std::isspace(c))
                continue;

            if (std::isdigit(c) || c == '.')
            {
                std::string numStr;
                while (i < expr.length() && (std::isdigit(expr[i]) || expr[i] == '.'))
                {
                    numStr += expr[i++];
                }
                --i;
                tokens.push_back({TokenType::Number, numStr});
            }
            else if (std::isalpha(c))
            {
                std::string func;
                while (i < expr.length() && std::isalpha(expr[i]))
                {
                    func += expr[i++];
                }
                --i;
                tokens.push_back({TokenType::Function, func});
            }
            else if (c == '(')
            {
                tokens.push_back({TokenType::LeftParen, "("});
            }
            else if (c == ')')
            {
                tokens.push_back({TokenType::RightParen, ")"});
            }
            else
            {
                if (c == '-')
                {
                    if (tokens.empty() || tokens.back().type == TokenType::LeftParen ||
                        tokens.back().type == TokenType::Operator)
                    {
                        tokens.push_back({TokenType::Function, "_"});
                    }
                    else
                    {
                        tokens.push_back({TokenType::Operator, "-"});
                    }
                }
                else
                {
                    tokens.push_back({TokenType::Operator, std::string(1, c)});
                }
            }
        }
        return tokens;
    }

    std::vector< Token > shuntingYard(const std::vector< Token >& tokens)
    {
        std::vector< Token > output;
        std::stack< Token >  operators;

        for (const auto& token : tokens)
        {
            if (token.type == TokenType::Number)
            {
                output.push_back(token);
            }
            else if (token.type == TokenType::Function)
            {
                operators.push(token);
            }
            else if (token.type == TokenType::Operator)
            {
                while (!operators.empty() && operators.top().type != TokenType::LeftParen)
                {
                    const auto& top = operators.top();
                    if (top.type == TokenType::Function ||
                        (!isRightAssociative(token.value) && getPrecedence(token.value) <= getPrecedence(top.value)) ||
                        (isRightAssociative(token.value) && getPrecedence(token.value) < getPrecedence(top.value)))
                    {
                        output.push_back(top);
                        operators.pop();
                    }
                    else
                    {
                        break;
                    }
                }
                operators.push(token);
            }
            else if (token.type == TokenType::LeftParen)
            {
                operators.push(token);
            }
            else if (token.type == TokenType::RightParen)
            {
                while (!operators.empty() && operators.top().type != TokenType::LeftParen)
                {
                    output.push_back(operators.top());
                    operators.pop();
                }
                if (operators.empty())
                    throw std::runtime_error("Mismatched parentheses");
                operators.pop();
                if (!operators.empty() && operators.top().type == TokenType::Function)
                {
                    output.push_back(operators.top());
                    operators.pop();
                }
            }
        }

        while (!operators.empty())
        {
            if (operators.top().type == TokenType::LeftParen)
                throw std::runtime_error("Mismatched parentheses");
            output.push_back(operators.top());
            operators.pop();
        }

        return output;
    }

    double solveRPN(const std::vector< Token >& rpn)
    {
        std::stack< double > stack;

        for (const auto& token : rpn)
        {
            if (token.type == TokenType::Number)
            {
                stack.push(std::stod(token.value));
            }
            else if (token.type == TokenType::Operator)
            {
                if (stack.size() < 2)
                    throw std::runtime_error("Invalid expression");
                double b = stack.top();
                stack.pop();
                double a = stack.top();
                stack.pop();

                if (token.value == "+")
                    stack.push(a + b);
                else if (token.value == "-")
                    stack.push(a - b);
                else if (token.value == "*")
                    stack.push(a * b);
                else if (token.value == "/")
                {
                    if (b == 0)
                        throw std::runtime_error("Division by zero");
                    stack.push(a / b);
                }
                else if (token.value == "^")
                    stack.push(std::pow(a, b));
            }
            else if (token.type == TokenType::Function)
            {
                if (stack.empty())
                    throw std::runtime_error("Invalid function call");
                double a = stack.top();
                stack.pop();
                if (token.value == "_")
                    stack.push(-a);
                else if (token.value == "sin")
                    stack.push(std::sin(a));
                else if (token.value == "cos")
                    stack.push(std::cos(a));
                else if (token.value == "tan")
                    stack.push(std::tan(a));
                else if (token.value == "sqrt")
                    stack.push(std::sqrt(a));
                else
                    throw std::runtime_error("Unknown function: " + token.value);
            }
        }

        if (stack.size() != 1)
            throw std::runtime_error("Invalid expression");
        return stack.top();
    }
};

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(600, 400, "Kalkulator", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    char        inputBuffer[256] = "";
    std::string resultText       = "Result: ";
    Calculator  calc;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin(
            "Calculator", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        ImGui::Text("Enter expression:");
        if (ImGui::InputText("##expr", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            try
            {
                double res = calc.evaluate(inputBuffer);
                resultText = "Result: " + std::to_string(res);
            }
            catch (const std::exception& e)
            {
                resultText = "Error: " + std::string(e.what());
            }
        }

        if (ImGui::Button("Calculate"))
        {
            try
            {
                double res = calc.evaluate(inputBuffer);
                resultText = "Result: " + std::to_string(res);
            }
            catch (const std::exception& e)
            {
                resultText = "Error: " + std::string(e.what());
            }
        }

        ImGui::Separator();
        ImGui::Text("%s", resultText.c_str());

        ImGui::Separator();
        ImGui::Text("Supported: +, -, *, /, ^, sin, cos, tan, sqrt, (, )");

        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}