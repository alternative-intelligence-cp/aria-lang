#!/usr/bin/env python3
"""
Tesla Package Manager - Core Implementation
Consciousness-validated software distribution for Tesla programming language
"""

import os
import sys
import json
import hashlib
import subprocess
import urllib.request
import tarfile
import tempfile
from pathlib import Path
from typing import Dict, List, Optional, Tuple
import argparse

# Tesla consciousness computing integration
sys.path.append(os.path.join(os.path.dirname(__file__), '..', 'src', 'python'))

class TeslaPackageManager:
    """Core Tesla Package Manager with consciousness validation"""
    
    def __init__(self):
        self.config_dir = Path.home() / '.tesla-pkg'
        self.packages_dir = self.config_dir / 'packages'
        self.cache_dir = self.config_dir / 'cache'
        self.registry_url = "https://registry.tesla-consciousness.org"
        self.pi_frequency = 3.141592653589793
        
        # Create directories if they don't exist
        self.config_dir.mkdir(exist_ok=True)
        self.packages_dir.mkdir(exist_ok=True)
        self.cache_dir.mkdir(exist_ok=True)
        
        # Load configuration
        self.config_file = self.config_dir / 'config.json'
        self.config = self._load_config()
        
    def _load_config(self) -> Dict:
        """Load Tesla package manager configuration"""
        default_config = {
            "registry_url": self.registry_url,
            "consciousness_validation": True,
            "pi_frequency": self.pi_frequency,
            "auto_update": False,
            "verify_signatures": True,
            "installed_packages": {}
        }
        
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    config = json.load(f)
                    # Merge with defaults
                    for key, value in default_config.items():
                        if key not in config:
                            config[key] = value
                    return config
            except (json.JSONDecodeError, IOError) as e:
                print(f"Warning: Could not load config: {e}")
                
        return default_config
    
    def _save_config(self):
        """Save configuration to disk"""
        try:
            with open(self.config_file, 'w') as f:
                json.dump(self.config, f, indent=2)
        except IOError as e:
            print(f"Error saving config: {e}")
    
    def _validate_consciousness(self, package_data: bytes) -> bool:
        """Validate package consciousness synchronization to π Hz"""
        if not self.config.get("consciousness_validation", True):
            return True
            
        # Calculate consciousness entropy using Tesla frequency
        entropy_hash = hashlib.sha256(package_data).hexdigest()
        
        # Convert hash to numerical value for consciousness calculation
        hash_value = int(entropy_hash[:16], 16)
        
        # Tesla consciousness validation: sin(π × entropy) ≈ π relationship
        import math
        normalized_entropy = (hash_value % 10000) / 10000.0
        consciousness_value = abs(math.sin(math.pi * normalized_entropy))
        
        # Package is consciousness-valid if it resonates with π frequency
        consciousness_threshold = 0.31415  # π/10 threshold
        is_valid = consciousness_value >= consciousness_threshold
        
        if is_valid:
            print(f"✅ Consciousness validation passed (π resonance: {consciousness_value:.5f})")
        else:
            print(f"❌ Consciousness validation failed (π resonance: {consciousness_value:.5f})")
            
        return is_valid
    
    def _verify_signature(self, package_path: Path, signature: str) -> bool:
        """Verify Tesla consciousness cryptographic signature"""
        if not self.config.get("verify_signatures", True):
            return True
            
        # Read package data for signature verification
        try:
            with open(package_path, 'rb') as f:
                package_data = f.read()
                
            # Calculate expected signature (simplified for demo)
            expected_signature = hashlib.sha256(
                package_data + str(self.pi_frequency).encode()
            ).hexdigest()
            
            is_valid = signature == expected_signature
            if is_valid:
                print("✅ Tesla consciousness signature verified")
            else:
                print("❌ Tesla consciousness signature verification failed")
                
            return is_valid
            
        except IOError as e:
            print(f"Error verifying signature: {e}")
            return False
    
    def install(self, package_name: str, version: Optional[str] = None) -> bool:
        """Install a Tesla consciousness package"""
        print(f"🧠⚡ Installing Tesla package: {package_name}")
        
        try:
            # Download package metadata
            package_info = self._fetch_package_info(package_name, version)
            if not package_info:
                print(f"❌ Package {package_name} not found in Tesla consciousness registry")
                return False
            
            # Download package file
            package_url = package_info['download_url']
            package_file = self._download_package(package_url, package_name)
            if not package_file:
                return False
            
            # Validate consciousness synchronization
            with open(package_file, 'rb') as f:
                package_data = f.read()
                
            if not self._validate_consciousness(package_data):
                print("❌ Package failed Tesla consciousness validation")
                return False
            
            # Verify cryptographic signature
            if not self._verify_signature(package_file, package_info.get('signature', '')):
                print("❌ Package signature verification failed")
                return False
            
            # Extract and install package
            install_path = self.packages_dir / package_name
            if not self._extract_package(package_file, install_path):
                return False
            
            # Update installed packages registry
            self.config['installed_packages'][package_name] = {
                'version': package_info['version'],
                'installed_date': __import__('datetime').datetime.now().isoformat(),
                'consciousness_validated': True,
                'install_path': str(install_path)
            }
            self._save_config()
            
            print(f"✅ Successfully installed {package_name} v{package_info['version']}")
            print(f"📍 Installed to: {install_path}")
            return True
            
        except Exception as e:
            print(f"❌ Installation failed: {e}")
            return False
    
    def _fetch_package_info(self, package_name: str, version: Optional[str] = None) -> Optional[Dict]:
        """Fetch package information from Tesla consciousness registry"""
        try:
            version_param = f"/{version}" if version else ""
            url = f"{self.config['registry_url']}/packages/{package_name}{version_param}"
            
            # For demo purposes, return mock package info with proper signature
            # In real implementation, this would fetch from actual registry
            
            # Pre-calculate what the mock package content will be
            mock_content = {
                'name': package_name,
                'tesla_consciousness': True,
                'pi_frequency': self.pi_frequency,
                'content': f'Mock Tesla consciousness package: {package_name}'
            }
            mock_content_bytes = json.dumps(mock_content, indent=2).encode()
            
            # Calculate signature for this exact content
            mock_signature = hashlib.sha256(
                mock_content_bytes + str(self.pi_frequency).encode()
            ).hexdigest()
            
            mock_package_info = {
                'name': package_name,
                'version': version or '1.0.0',
                'description': f'Tesla consciousness-validated package: {package_name}',
                'download_url': f"{self.config['registry_url']}/downloads/{package_name}.tpkg",
                'signature': mock_signature,
                'dependencies': [],
                'consciousness_frequency': self.pi_frequency,
                'tesla_compatible': True
            }
            
            print(f"📦 Found package: {package_name} v{mock_package_info['version']}")
            return mock_package_info
            
        except Exception as e:
            print(f"Error fetching package info: {e}")
            return None
    
    def _download_package(self, url: str, package_name: str) -> Optional[Path]:
        """Download Tesla package file"""
        try:
            print(f"📥 Downloading {package_name}...")
            
            # For demo purposes, create a mock package file
            # In real implementation, this would download from actual URL
            package_file = self.cache_dir / f"{package_name}.tpkg"
            
            # Create mock package content with signature-compatible data
            mock_content = {
                'name': package_name,
                'tesla_consciousness': True,
                'pi_frequency': self.pi_frequency,
                'content': f'Mock Tesla consciousness package: {package_name}'
            }
            
            content_bytes = json.dumps(mock_content, indent=2).encode()
            
            with open(package_file, 'wb') as f:
                f.write(content_bytes)
            
            print(f"✅ Downloaded to {package_file}")
            return package_file
            
        except Exception as e:
            print(f"Error downloading package: {e}")
            return None
    
    def _extract_package(self, package_file: Path, install_path: Path) -> bool:
        """Extract Tesla package to installation directory"""
        try:
            print(f"📂 Extracting package to {install_path}...")
            
            # Create installation directory
            install_path.mkdir(parents=True, exist_ok=True)
            
            # For demo purposes, just copy the package file
            # In real implementation, this would extract .tpkg archive
            import shutil
            shutil.copy2(package_file, install_path / 'package.json')
            
            # Create Tesla consciousness manifest
            manifest = {
                'package_format': 'tesla_consciousness_package',
                'pi_frequency': self.pi_frequency,
                'consciousness_validated': True,
                'extraction_date': __import__('datetime').datetime.now().isoformat()
            }
            
            with open(install_path / 'TESLA_MANIFEST.json', 'w') as f:
                json.dump(manifest, f, indent=2)
            
            print("✅ Package extracted successfully")
            return True
            
        except Exception as e:
            print(f"Error extracting package: {e}")
            return False
    
    def list_installed(self) -> Dict:
        """List all installed Tesla consciousness packages"""
        return self.config.get('installed_packages', {})
    
    def remove(self, package_name: str) -> bool:
        """Remove an installed Tesla consciousness package"""
        if package_name not in self.config['installed_packages']:
            print(f"❌ Package {package_name} is not installed")
            return False
        
        try:
            package_info = self.config['installed_packages'][package_name]
            install_path = Path(package_info['install_path'])
            
            # Remove package directory
            if install_path.exists():
                import shutil
                shutil.rmtree(install_path)
                print(f"🗑️  Removed package files from {install_path}")
            
            # Remove from installed packages registry
            del self.config['installed_packages'][package_name]
            self._save_config()
            
            print(f"✅ Successfully removed {package_name}")
            return True
            
        except Exception as e:
            print(f"❌ Error removing package: {e}")
            return False
    
    def health_check(self) -> bool:
        """Check Tesla consciousness system health"""
        print("🧠⚡ Tesla Consciousness System Health Check ⚡🧠")
        print("=" * 50)
        
        issues_found = 0
        
        # Check configuration
        print("📋 Configuration:")
        print(f"  ✅ Config directory: {self.config_dir}")
        print(f"  ✅ Packages directory: {self.packages_dir}")
        print(f"  ✅ Cache directory: {self.cache_dir}")
        print(f"  ✅ π frequency: {self.config['pi_frequency']}")
        print(f"  ✅ Consciousness validation: {self.config['consciousness_validation']}")
        
        # Check installed packages
        installed = self.list_installed()
        print(f"\n📦 Installed Packages: {len(installed)}")
        
        for name, info in installed.items():
            install_path = Path(info['install_path'])
            if install_path.exists():
                print(f"  ✅ {name} v{info['version']}")
            else:
                print(f"  ❌ {name} v{info['version']} (missing files)")
                issues_found += 1
        
        # System health summary
        print(f"\n🎯 Health Summary:")
        if issues_found == 0:
            print("  ✅ All systems operating at optimal Tesla consciousness frequency")
            print(f"  🧠 π Hz synchronization: PERFECT")
            return True
        else:
            print(f"  ⚠️  {issues_found} issue(s) detected")
            print("  🔧 Run 'tesla-pkg cleanup' to repair")
            return False


def main():
    """Tesla Package Manager CLI interface"""
    parser = argparse.ArgumentParser(
        description='Tesla Package Manager - Consciousness-validated software distribution',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  tesla-pkg install neural-sync-lib    Install consciousness-validated package
  tesla-pkg list                       List installed packages  
  tesla-pkg remove old-package         Remove package cleanly
  tesla-pkg health                     Check system health
  
🧠⚡ Tesla Consciousness Computing - π Hz synchronized software distribution ⚡🧠
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Install command
    install_parser = subparsers.add_parser('install', help='Install Tesla consciousness package')
    install_parser.add_argument('package', help='Package name to install')
    install_parser.add_argument('--version', help='Specific version to install')
    
    # Remove command
    remove_parser = subparsers.add_parser('remove', help='Remove installed package')
    remove_parser.add_argument('package', help='Package name to remove')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List installed packages')
    
    # Health command
    health_parser = subparsers.add_parser('health', help='Check system health')
    
    # Parse arguments
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    # Initialize Tesla Package Manager
    tpm = TeslaPackageManager()
    
    # Execute command
    if args.command == 'install':
        success = tpm.install(args.package, args.version)
        return 0 if success else 1
        
    elif args.command == 'remove':
        success = tpm.remove(args.package)
        return 0 if success else 1
        
    elif args.command == 'list':
        installed = tpm.list_installed()
        if installed:
            print("🧠⚡ Installed Tesla Consciousness Packages ⚡🧠")
            print("=" * 50)
            for name, info in installed.items():
                print(f"📦 {name} v{info['version']}")
                print(f"   📅 Installed: {info['installed_date']}")
                print(f"   ✅ Consciousness validated: {info['consciousness_validated']}")
                print()
        else:
            print("No Tesla consciousness packages installed")
        return 0
        
    elif args.command == 'health':
        healthy = tpm.health_check()
        return 0 if healthy else 1
    
    else:
        print(f"Unknown command: {args.command}")
        return 1


if __name__ == '__main__':
    sys.exit(main())