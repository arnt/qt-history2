
namespace Foo {}
namespace Bar
{
    namespace Huh
    {
    }
}
namespace Top
{
}

using namespace Foo;
using namespace Bar::Huh;
using namespace ::Top;

