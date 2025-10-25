# HueOS Versioning Scheme

HueOS follows [Semantic Versioning](https://semver.org/) (SemVer) for version numbering.

## Version Format

```
MAJOR.MINOR.PATCH
```

- **MAJOR** version: Incompatible changes, major architectural changes, or significant feature sets
- **MINOR** version: New features, backward-compatible functionality additions
- **PATCH** version: Bug fixes, documentation updates, minor improvements

## Version History

### v2.2.0 (Current - October 2025)
**Project Organization & Build System Improvements**
- Clean project structure with organized documentation
- ISO build output moved to `build/` directory
- Added `package` and `preserve` targets for distribution
- Comprehensive Hyper-V documentation (Gen 1 & Gen 2)
- Historical documentation archived
- Professional OSS project layout

### v2.1.0 (October 2025)
**SCSI Storage Support**
- SCSI storage driver implementation
- LSI Logic 53C895A controller support
- BusLogic controller framework
- PCI bus scanning for SCSI controllers
- Hyper-V Generation 2 VM compatibility

### v2.0.0 (October 2025)
**UEFI Boot Support**
- UEFI boot support via GRUB2
- Hybrid BIOS + UEFI bootable ISO
- Support for both x86_64-efi and i386-efi platforms

### Earlier Versions
Prior to v2.0.0, the project focused on:
- Video mode support (80x25, 80x50, 132x25, 132x50)
- Framebuffer text rendering
- VGA and VESA graphics support
- Basic kernel initialization

## Release Process

### Creating a New Release

1. **Update VERSION file**
   ```bash
   echo "X.Y.Z" > VERSION
   ```

2. **Update documentation**
   - Update this file with release notes
   - Update `README.md` if needed
   - Document breaking changes (for MAJOR releases)

3. **Commit version bump**
   ```bash
   git add VERSION docs/VERSIONING.md
   git commit -m "Bump version to vX.Y.Z"
   ```

4. **Create and push tag**
   ```bash
   git tag -a vX.Y.Z -m "Release vX.Y.Z - Brief Description

   Major Features:
   - Feature 1
   - Feature 2
   - Feature 3"
   
   git push origin main
   git push origin vX.Y.Z
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

## Version Increment Guidelines

### MAJOR Version (X.0.0)
Increment when you make:
- Incompatible boot process changes
- Major kernel API changes
- Significant architectural redesigns
- Removal of deprecated features

### MINOR Version (x.Y.0)
Increment when you add:
- New hardware support (new drivers)
- New boot modes or platforms
- New command-line features
- Backward-compatible kernel extensions

### PATCH Version (x.y.Z)
Increment when you make:
- Bug fixes
- Documentation updates
- Performance improvements
- Code refactoring (no feature changes)
- Build system improvements (if minor)

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

## Pre-release Versions

For development or testing releases, append a pre-release identifier:

```
X.Y.Z-alpha.N    # Alpha releases
X.Y.Z-beta.N     # Beta releases
X.Y.Z-rc.N       # Release candidates
```

Example:
```bash
git tag -a v2.3.0-beta.1 -m "Release v2.3.0-beta.1 - Testing new filesystem support"
```

## Notes

- Always update the `VERSION` file when creating a new release
- Keep this document updated with release notes for each version
- Use annotated tags (`-a`) for releases, not lightweight tags
- Push tags separately: `git push origin --tags`
- Create GitHub Releases for user-facing version downloads
