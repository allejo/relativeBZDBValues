# Relative BZDB Values

[![GitHub release](https://img.shields.io/github/release/allejo/relativeBZDBValues.svg)](https://github.com/allejo/relativeBZDBValues/releases/latest)
![Minimum BZFlag Version](https://img.shields.io/badge/BZFlag-v2.4.4+-blue.svg)
[![License](https://img.shields.io/github/license/allejo/relativeBZDBValues.svg)](LICENSE.md)

A BZFlag plug-in which will change BZDB variables based on the amount of non-observer players that are currently on the server.

## Requirements

- BZFlag 2.4.4+
- C++11
- [yaml-cpp 0.5+](https://github.com/jbeder/yaml-cpp/)

## Usage

### Compiler Requirements

On Ubuntu/Debian distributions, you will need to install the `libyaml-cpp-dev` and `libboost-dev` packages. Or you may build/install `yaml-cpp` 0.6.x from its source and skip the Boost dependency.

### Loading the plug-in

This plug-in requires a single command line argument, the path to the YAML configuration file.

```
-loadplugin relativeBZDBValues,/path/to/relativeBZDBValues.yml
```

### Configuration File

If the plugin requires a custom configuration file, describe it here and all of its special values

```yaml
# The "namespace" the plug-in will read from
relative_bzdb:
    - bzdb: _tankSpeed # The BZDB variable that will be set
      delay: 60        # Number of seconds between BZDB changes
      values:
          - minPlayers: 1 # The minimum amount of non-observer players needed for this condition to apply
            intValue: 40  # The value that will set
            message: "As a lone tank, _tankSpeed has been set 40" # (Optional) Message to announce changes
          - minPlayers: 2
            intValue: 35
            message: "Tank speed has been set to 35"
```

Each item inside the `values` array may contain the following value types depending on the BZDB variable being set.

- `intValue`
- `boolValue`
- `doubleValue`
- `stringValue`

For example, creating a rule for changing the `_skyColor` based on player count would be the following:

```yaml
- bzdb: _skyColor
  delay: 60
  values:
      - minPlayers: 1
        stringValue: red
      - minPlayers: 2
        stringValue: blue
      - minPlayers: 3
        stringValue: green
```

### Custom Slash Commands

| Command | Permission | Description |
| ------- | :--------: | ----------- |
| `/reload` | `setAll` | This plug-in overloads `/reload` to reload the configuration file. This does not stop propagation and allows other plugins and the server to reload what's needed. |
| `/reload all` | `setAll` | Alias for `/reload` without any options. |
| <code>/reload&nbsp;relativeBZDB</code> | `setAll` | This plug-in overloads `/reload` to insert an additional option to reload *just* the configuration of this plug-in: `relativeBZDB`. |
| `/set` | `setAll` `setVar` | This plug-in overloads the `/set` command to disallow setting BZDB variables if they're maintained by this plug-in. |

## License

[MIT](LICENSE.md)
