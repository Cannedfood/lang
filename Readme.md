# Lang (placeholder name)

This is a scripting language with focus on (in this order)
1. Being easy to embed
2. Convenience
   1. Coroutines and stream operations
3. Performance

# Tutorial
```python
# Classes
class Animal
	get location # Readonly member variable

class Bird extends Animal
	constr()
		.location = "nest"

	flyTo(destination)
		.location = destination

var flock = [Bird(), Bird(), Bird()]
```

# Ideas
Modules:
	- Resolution: Relative, Default module path, -Iarguments/
Stream operators using |
	```js
	co sort(items) -> int {
		items = items | to_array

	}

	["c", "d", "e"] | .length | print
	```
Ternary operator:
	def a = cond? whe_true:when_false;
	cond? when_true();
