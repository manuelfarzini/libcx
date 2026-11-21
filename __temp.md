- Bracing break:
  AfterFunction: true
  AfterClass: true
  AfterStruct: true
  AfterEnum: true
  AfterControlStatement: Never
  AfterNamespace: false
  AfterObjCDeclaration: false
  AfterUnion: true
  AfterCaseLabel: false
  AfterExternBlock: false
  BeforeCatch: false
  BeforeElse: false
  BeforeLambdaBody: false
  BeforeWhile: false
  SplitEmptyFunction: true
  SplitEmptyRecord: false

  - traits, concepts and other similar utilities. can be special cases since they're struct but we don't
    break before the brace.

- Free Function-signature formatting: switch to multi-line when 

  ```
  <teamplates>
  <attributes> fn func_name(args..) noexce -> Ret with <constraints>
  {
  }
  ```
  exceedes the 100 char limit. Here the possible cases:

  - only exceedes the `with` clause, then it is moved to newline alone with 4 spaces indentation.
  - otherwise we go for:

    ```
    <templates>
    <attributes> fn func_name(
        args..
    ) noexce -> Ret with (<constraints>) {
    ```

  - if the constraints don't fit either:

    ```
    <templates>
    <attributes> fn func_name(
        args..
    ) noexce -> Ret
      with (<constraints>) {
    ```

  - if needed:

    ```
    <templates>
    <attributes> fn func_name(
        args..
    ) noexce -> Ret
        with (<constraint>
              <constraint>)
    {
    ```

  - for templates, if needed.

    ```
    template<
        para..
    >
    <attributes> fn func_name(
        args..
    ) noexce -> Ret
      with (<constraints>) {
    ``` 

- Template formatting: 

  - always use with outside the angled brace.

  - keep the constraints inline if short, if the totale exceedes go directly with:
  
  ```
  template<
      para..
  > with <constraints>
  struct Name
  {
      ...
  };
  ```

  - `template` should be on his own line but in concepts and traits and other similar cases we use a single
    line

  
- Alignment: 
  align arguments, parameters, enum options, enum like constants, related struct fields. That's it, do not
  align in other places

- Error handling: 

    - if possible, always use `or_return`, and `or_else` instead of the ordinary `if(err)`

    - `cx_assert` is a security/optimization thing.
      - well we have to put in the styling options that `cx_assert` and `cx_debug` will be eventually
        disabled in release, where `cx_panic` and `cx_assert_hard` (that expands in panic too) will remain
        active.
      - I mean, we can use `cx_assert` with 2 purposes: enstrong a critical invariant and check for critical
        errors that we are evenutally shure that don't happen in release, and so we do not want to branch,
        but we wanna go straight and faster.


- Concept formatting: provide explicit examples of inline traits, composed concepts, and multiline requires.
- Parameter passing: establish guidelines for values, mutable references, const references, and pointers.
- Function contracts: standardize documentation and distinguish @req from genuinely recoverable error conditions.
- Initialization and ownership: formalize the distinction between raw storage, byte zeroing, value initialization, and explicit cleanup.
- Comments: decide when documentation is necessary and standardize markers.
- Implementation complexity: clarify the preference for straightforward code and when an abstraction is actually justified.
- Code organization: establish a preferred ordering of declarations within each header.
