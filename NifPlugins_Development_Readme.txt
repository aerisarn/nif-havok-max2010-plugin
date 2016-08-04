                  NifPlugins for 3ds Max
                  ======================

    ATTENTION:
    ----------
    This is not the officially maintained edition of the NifTools plugin
    
    
    Description
    -----------    
    This document describes the how to setup your build environment to use this 
     project.

        
    Enjoy!

    
    Requirements
    ------------
      Microsoft Visual Studio 2015
        - Visual C++ Express Edition is sufficient

    Installation
    ------------
    1. Check out max_nif_plugin from github.com
      git clone --recursive git://github.com/figment/max_nif_plugin.git max_nif_plugin
      git remote add upstream git://github.com/niflib/max_nif_plugin.git
      
    2. In the same root folder checkout niflib from github.com
      git clone --recursive git://github.com/figment/niflib.git niflib
      git remote add upstream git://github.com/niflib/niflib.git
    
    3. If you branch these then you need to fix the submodules in niflib
       and that is not a easy to use workflow
            
    4. Edit User_Settings.props to match 3ds max SDK installation directories
    
    5. Run makeconfig.bat from command line with git and sed available
       Important as it creates the NifCommon\config.h file with version info
    
    6. To build 3ds Max 2016, select "Release - Max 2016"  and "x64" and build
    
    
 
    Copyright
    ---------
      
    Copyright (c) 2015, NIF File Format Library and Tools. All rights reserved.
    
    Legal
    -----
      
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
    FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
    COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
