# Lang (placeholder name)

This is a scripting language with focus on (in this order)
1. Being easy to embed
2. Convenience
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
