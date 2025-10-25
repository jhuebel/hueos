# HueOS Versioning Scheme

HueOS follows [Semantic Versioning](https://semver.org/) (SemVer) for version numbering.

## Current Status

**HueOS is currently in ALPHA stage** - This means the OS is under active development with core features being implemented and tested. Breaking changes may occur between releases.

## Version Format

```
0.MINOR.PATCH-alpha
```

During the alpha phase:
- **MAJOR** version is `0` (indicating pre-1.0 alpha software)
- **MINOR** version: Significant feature additions or milestones
- **PATCH** version: Bug fixes, minor improvements, documentation updates
- **-alpha** suffix: Indicates alpha/experimental status

Once the OS reaches a stable feature set and API, it will move to:
- **v0.x.x-beta** for beta testing
- **v1.0.0** for the first stable release

## Version History

### v0.3.0-alpha (Current - October 2025)
**Project Organization & Build System Improvements**
- Clean project structure with organized documentation
- ISO build output moved to `build/` directory
- Added `package` and `preserve` targets for distribution
- Comprehensive Hyper-V documentation (Gen 1 & Gen 2)
- Historical documentation archived
- Professional OSS project layout
- Versioning scheme established

### v0.2.0-alpha (October 2025)
**SCSI Storage Support**
- SCSI storage driver implementation
- LSI Logic 53C895A controller support
- BusLogic controller framework
- PCI bus scanning for SCSI controllers
- Hyper-V Generation 2 VM compatibility

### v0.1.0-alpha (October 2025)
**UEFI Boot Support**
- UEFI boot support via GRUB2
- Hybrid BIOS + UEFI bootable ISO
- Support for both x86_64-efi and i386-efi platforms

### Earlier Development
Prior to v0.1.0-alpha, the project focused on:
- Video mode support (80x25, 80x50, 132x25, 132x50)
- Framebuffer text rendering
- VGA and VESA graphics support
- Basic kernel initialization

## Release Process

### Creating a New Alpha Release

1. **Update VERSION file**
   ```bash
   echo "0.X.Y-alpha" > VERSION
   ```

2. **Update documentation**
   - Update this file with release notes
   - Update `README.md` if needed
   - Document any breaking changes

3. **Commit version bump**
   ```bash
   git add VERSION docs/VERSIONING.md
   git commit -m "Bump version to v0.X.Y-alpha"
   ```

4. **Create and push tag**
   ```bash
   git tag -a v0.X.Y-alpha -m "Release v0.X.Y-alpha - Brief Description

   Major Features:
   - Feature 1
   - Feature 2
   - Feature 3
   
   Note: This is an alpha release for testing and development."
   
   git push origin main
   git push origin v0.X.Y-alpha
   ```

5. **Create distribution package**
   ```bash
   make package
   ```

6. **Create GitHub Release**
   - Go to GitHub repository
   - Navigate to Releases
   - Click "Create a new release"
   - Select the tag
   - Add release notes
   - Attach `dist/hueos.iso` and `dist/hueos.bin`

## Version Increment Guidelines (Alpha Stage)

### MINOR Version (0.X.0-alpha)
Increment when you add:
- New hardware support (new drivers)
- New boot modes or platforms
- Significant feature milestones
- Major subsystem implementations

### PATCH Version (0.x.Y-alpha)
Increment when you make:
- Bug fixes
- Documentation updates
- Performance improvements
- Code refactoring (no feature changes)
- Minor build system improvements

### Moving Beyond Alpha

**Beta Stage (0.x.x-beta):**
- Core functionality is stable
- API is mostly finalized
- Focus shifts to bug fixes and polish
- Breaking changes are rare

**Stable Release (1.0.0):**
- All core features implemented
- API is stable and documented
- Comprehensive testing completed
- Ready for production use

After v1.0.0, follow standard SemVer:
- **MAJOR (X.0.0)**: Breaking changes, major architectural changes
- **MINOR (x.Y.0)**: New features, backward-compatible
- **PATCH (x.y.Z)**: Bug fixes, minor improvements

## Viewing Version Information

### Check current version
```bash
cat VERSION
```

### List all tags
```bash
git tag -l
```

### View tag details
```bash
git show vX.Y.Z
```

### View version in releases
GitHub Releases page: https://github.com/jhuebel/hueos/releases

## Pre-release Identifiers

### Alpha Releases (Current)
```
0.X.Y-alpha    # Main alpha releases
```

HueOS is currently in alpha. All releases use the `-alpha` suffix to indicate:
- Active development
- Experimental features
- Possible breaking changes
- Testing and feedback phase

### Future Pre-release Formats

**Beta releases** (when ready):
```
0.X.Y-beta.N    # Beta releases with iteration number
```

**Release candidates** (before 1.0.0):
```
1.0.0-rc.N      # Release candidates before stable
```

Example progression:
```
0.3.0-alpha → 0.4.0-alpha → 0.5.0-beta.1 → 0.5.0-beta.2 → 1.0.0-rc.1 → 1.0.0
```

## Notes

- Always update the `VERSION` file when creating a new release
- Keep this document updated with release notes for each version
- Use annotated tags (`-a`) for releases, not lightweight tags
- Push tags separately: `git push origin --tags`
- Create GitHub Releases for user-facing version downloads
