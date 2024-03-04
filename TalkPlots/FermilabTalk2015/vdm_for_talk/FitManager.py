# idea from http://martyalchin.com/2008/jan/10/simple-plugin-framework
# other helping knowledge 
# http://www.voidspace.org.uk/python/articles/five-minutes.shtml

##### plugin framework########

class PluginMount(type):
    def __init__(cls,name,base,attrs):
        if not hasattr(cls,'plugins'):
        # This branch only executes when processing the mount point itself
        # So, since this is a new plugin type, not an implementation, this
        # class shouldn't be registered as a plugin. Instead, it sets up a
        # list where plugins can be regisstered later.
            cls.plugins = [] 
        else:
        # This must be a plugin implementation, which should be registered.
        # Simply appending it to the list is all that's needed to keep
        # track of it later.
            cls.plugins.append(cls)

##### example plugin usecase #####
#
# Declaring a mount point base class
#
class FitProvider:
    '''
    Mount point for plugins which reger to actions that can be performed.
    '''
    __metaclass__ = PluginMount 

    #FitProvider is a class, but instead of being an instance of type, the class object is now an instance of our PluginMount metaclass
    #It can now serve as a mount point for plugins that provide actions.
#
# Implementation of two plugins
#

def get_plugins(cls):
    pluginnames = [p.__name__ for p in cls.plugins]
    plugins = [p for p in cls.plugins]
    return dict(zip(pluginnames,plugins))
