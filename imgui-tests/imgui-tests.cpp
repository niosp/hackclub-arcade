// imgui-tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include <imgui.h>

int main()
{
    ImGui::CreateContext();
    ImGui::NewFrame();
    char buf[20];
    float f;
    ImGui::Text("Hello, world %d", 123);
    if (ImGui::Button("Save"))
        std::cout << "saved" << std::endl;
    ImGui::InputText("string", buf, IM_ARRAYSIZE(buf));
    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
    std::cout << "Hello World!\n";
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
