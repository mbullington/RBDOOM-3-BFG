# Coding guide

It's probably good (as I'm learning C++ more in-depth) to define good practices, and see when/if we can automate linting them.

Formatting is automatically handled with `clang-format` based on the Google styleguide.

Resources: [id Tech 4 coding guide](https://fabiensanglard.net/fd_proxy/doom3/CodeStyleConventions.pdf)

## STL Containers

Avoid using STL containers if you can! For external libraries this usually isn't tenable.

In the early 2000s there was a lot of concern around performance stability of the STL, so most game companies (including id) rolled their own. A lot of companies still do.

This rule is mostly to keep consistency within the codebase, as avoiding STL containers is the path of least resistance.

## Comments

The id Tech 4 guide writes comments like this, which feels redundant:

```c++
/*
==============
Sys_EXEPath
==============
*/
const char* Sys_EXEPath() {
}
```

New code should write comments sparsely and when necessary, like so:


```c++
// Return the EXE path on Windows.
//
// Note: The EXEPath will return the .app directory on macOS, and the directory
// of the ELF executable on Linux.
const char* Sys_EXEPath() {
}
```

## Classes 


Old class names start with "id" and each successive word starts with an
upper case.

```c++
class idVec3;
```

New code should be written using the `id` namespace instead, like so:

```c++
namespace id {

class Vec3;

} // namespace id
```

## Copied from id Tech 4 coding guide

Typedef names use the same naming convention as variables, however
they always end with "_t".

```c++
typedef int fileHandle_t;
```

Struct names use the same naming convention as variables, however
they always end with "_t".

```c++
struct renderEntity_t;
```

Enum names use the same naming convention as variables, however they
always end with "_t". The enum constants use all upper case
characters. Multiple words are separated with an underscore.

```c++
enum contact_t {
CONTACT_NONE,
CONTACT_EDGE,
CONTACT_MODELVERTEX,
CONTACT_TRMVERTEX
};
```

Names of recursive functions end with "_r"

```c++
void WalkBSP_r( int node );
```

Class methods have the same naming convention as functions.

```c++
class idVec3 {
float Length( void ) const;
}
```

