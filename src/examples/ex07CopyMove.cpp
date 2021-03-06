#include <giga/Application.h>
#include <giga/core/Uploader.h>
#include <cpprest/details/basic_types.h>

using giga::Config;
using giga::Application;
using giga::core::Uploader;
using giga::core::Node;
using giga::core::FolderNode;
using boost::filesystem::path;

using utility::string_t;

void printChildrenNodes(Node& node, string_t space = U(""))
{
    for (auto& child : node.getChildren())
    {
        ucout << space << child->name() << U("\n");
        printChildrenNodes(*child, space + U(" "));
    }
}

int main(int, char**)
{
    Config::init(string_t(U("http://localhost:5001")),
                    string_t(U("1142f21cf897")),
                    string_t(U("65934eaddb0b233dddc3e85f941bc27e")));

    Application app;
    auto owner = app.authenticate(U("test_main"), U("password"));
    ucout << U("Hello ") << owner.login() << U(" your id is ") << owner.id() << std::endl;

    auto root = owner.contactData().node();
    auto originCp   = root.createChildFolder(U("originCp"));
    auto originMv   = root.createChildFolder(U("originMv"));
    auto dest = root.createChildFolder(U("dest"));

    originCp.copyTo(dest);
    originMv.moveTo(dest);

    return 0;
}
