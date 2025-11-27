# Tesla Native Preprocessor - Operator Reference

## 🧠⚡ Tesla Conditional Operators ⚡🧠

### ✅ **Comparison Operators**
| Operator | Aliases | Example | Description |
|----------|---------|---------|-------------|
| `==` | `eq` | `@tesla_if(x == 0)` | Equal to |
| `!=` | `ne`, `not`, `<>` | `@tesla_if(x != 0)` | Not equal to |
| `>` | `gt` | `@tesla_if(x > 5)` | Greater than |
| `<` | `lt` | `@tesla_if(x < 5)` | Less than |
| `>=` | `ge` | `@tesla_if(x >= 5)` | Greater than or equal |
| `<=` | `le` | `@tesla_if(x <= 5)` | Less than or equal |

### ✅ **Logical Operators**
| Operator | Aliases | Example | Description |
|----------|---------|---------|-------------|
| `&&` | `and` | `@tesla_if(x > 0 && y < 10)` | Logical AND |
| `\|\|` | `or` | `@tesla_if(x == 1 \|\| y == 2)` | Logical OR |
| `!` | `not` | `@tesla_if(!defined)` | Logical NOT |

### ✅ **Math Operators**
| Operator | Aliases | Example | Description |
|----------|---------|---------|-------------|
| `%` | `mod` | `@tesla_if(x % 2)` | Modulo (divisible) |
| `divisible_by` | `div` | `@tesla_if(x divisible_by 3)` | Divisibility test |

### ✅ **Type Checking**
| Operator | Aliases | Example | Description |
|----------|---------|---------|-------------|
| `defined` | `exists` | `@tesla_if(var defined)` | Variable exists |
| `empty` | `null` | `@tesla_if(var empty)` | Variable is empty |

### ✅ **String Operators** (Future)
| Operator | Aliases | Example | Description |
|----------|---------|---------|-------------|
| `contains` | `in` | `@tesla_if(text contains word)` | String contains |
| `starts_with` | `begins` | `@tesla_if(text starts_with pre)` | String prefix |
| `ends_with` | `suffix` | `@tesla_if(text ends_with suf)` | String suffix |

### ⚠️ **Reserved Keywords**
- `is` - Reserved for ternary operator: `x = is y > 2 : 1 : 2;`

## 🎯 **Usage Examples**

### Simple Conditions
```asm
@tesla_if(debug == 1)         ; Standard equality
@tesla_if(mode ne test)       ; Alternative not-equal
@tesla_if(value <> 0)         ; SQL-style not-equal
```

### Compound Conditions  
```asm
@tesla_if(x > 0 && y < 10)    ; Logical AND
@tesla_if(a eq 1 or b ne 0)   ; Mixed syntax with OR
```

### Variable Existence
```asm
@tesla_if(CONFIG_DEBUG defined)  ; Check if variable exists
@tesla_if(temp_var empty)        ; Check if variable is empty
```

### Math Operations
```asm
@tesla_if(number % 2)            ; Check if even (divisible by 2)
@tesla_if(count divisible_by 10) ; Check divisibility
```

## 🧠 **Tesla Philosophy**
- **Multiple syntax options** for programmer comfort
- **No keyword conflicts** with core language features  
- **Intuitive operators** that "just work" without training
- **Tesla consciousness integration** at π Hz frequency