bref:
NYT Spelling Bee Solver in C++

verbeux:
An attempt to generate a solution for NYT Spelling Bee type problems with minimal execution time.
Currently completes in ~30ms with default compiler optimizations (-O0) reading an ~8000 entry dictionary

Why ...

... c++? I am not good at it but I would like to be.

... do I continue to let the knicks do this to me? Unclear. (5/31/2025 but likely this sentiment shall recur)

... bother?
    Obviously, "telling the computer what you want to solve and letting it solve the problem for you"
    is something which you could always *argue* is the case. Nevertheless, it feels to me as though
    to have any satisfaction in "solving" a problem requires some level of (at minimum) algebra (in the 
    most general sense) wherein I, the human, should be breaking down the problem somewhat on my own, the idea
    being that a computer can solve problems faster if you give it problems which are closer to being solved. Ideally,
    the problem is completely solved, and I can use the computer as a glorified verification scheme for an
    "algebraic" (really playing fast and loose with that term) type of solution. To me, this would look like
    code without branches. Of course, we use if;else branching often because it makes the code more legible, not
    for performance reasons, and keeping this in mind is good because it forces you to try and create solutions which
    are both branchless *and* legible, which would ideally correspond with more robust solutions.

As such, in an effort to optimize for execution time, the primary thing I've actually *done* is break down how
the problem is approached. For starters: the spelling bee, as the name implies, is an english-language definition-agnostic
spelling problem. We can define the starting point of an analagous problem to it as a set of 7 alphabetic characters, or,
somewhat crudely, 

S ⊂ {alphabet} where |S| = 7 

This set contains a subset K which consists of a single character. In the most traditional form, this character is
shown highlighted yellow in a central hexagon, with all other members of S shown in grey hexagons adjacent to the 6 
edges of the yellow hexagon. Solutions to the problem are dictionary words which contain K where all characters 
in the word are within set S, or, crudely again,

{solutions} ⊂ {dictionary words}

solutions = {word | (letters ⊆ word) ⊆ S ∧ K ⊂ (letters ⊆ word)}

Janky but it lays the groundwork for a more efficient approach to the problem. Crucially for my execution method, 
all the information needed to verify a single solution can be contained in <32 bits, which I use as follows:

0, (5)k, (26)a, or, 5 bits which identify the single character which makes up K, and 26 bits which identifies S.
All possible values of K and S can therefore be identified with this string of 31 bits.

ui32 is a fundamental datatype, so I used that to contain the verification map. The 2^32 place is always 0, but that's ok,
it's not too much of a waste.

To verify a solution, we represent a candidate as the set of letters contained within it over 26 bits. Using the 5 K bits, 
we verify the bit at candidate[K] is set to 1. We can do this by generating a new 26-bit map "check" where only bit[K] == 1,
taking the intersection of check ∧ candidate (which we can accomplish with bitwise-or &) and dividing the intersection by check. 
Doing this will output '1' if K ⊂ candidate or '0' otherwise. Let result_1 represent the result of this equation. Next, let U be
the union candidate ∪ S. If U != S, the candidate word contains at least one invalid character and is thus invalid. This can be 
accomplished using bitand for the union, and dividing the initial candidate by the union, which outputs '0' if the union is greater than
(and therefore outside the bounds of) S, or '1' if they are equivalent. The logic-and of these two statements lets us filter out
invalid words by setting them to 0 without having to branch.

Using this strategy means that the program can evaluate a single candidate in ~100ns, which is around as fast as can be accurately
measured on my current setup.