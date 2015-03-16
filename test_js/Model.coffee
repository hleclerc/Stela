class Model
    # static attributes
    @__counter: 0  # nb "change rounds" since the beginning ( * 2 to differenciate direct and indirect changes )
    @__modlist: new Set # models changed models during the current round
    @__n_views: [] # new views created during the current round (that will need a first onchange call)
    @__timeout: undefined # timer used to create a new "round" (to call View.on_change)
    @__force_m: false # if __force_m == true, every has_been_modified function will return true (used to call onchange of new views)
    @__cur_id : 1 # current model id (the one that will be used for the next created base model)
    
    constructor: ( @__ptr = asm_mod.allocate( 1 ), parent ) ->
        if parent?
            @__parent = parent
        else
            @__pardeps                = new Set # -> objects that directly depends on this
            @__views                  = new Set # views observing this
            @__id                     = 0 # global id (local at the beginning, if not saved in SocaDB)
            @__date_last_modification = 0 # Model.current_date+2 for a direct modification. Model.current_date+1 for a modification that comes from a child.

    #
    bind: ( view, onchange_construction = true ) ->
        if view instanceof View
            @__container().__views.add view

            if onchange_construction
                Model.__n_views.push view
                Model.__need_sync_views()
        else
            new FunctionBinder this, onchange_construction, f
            
    #
    unbind: ( view ) ->
        @__container().__views.delete view

    # attr_names -> list with name of attributes. May be surdefined
    Object.defineProperty Model.prototype, "attr_names",
        get: -> @get_attr_names()

    Object.defineProperty Model.prototype, "size_in_bits",
        get: -> @get_size_in_bits()

    Object.defineProperty Model.prototype, "val",
        get: ->
            @get_val()
        set: ( val ) ->
            if @set_val val
                @__signal_change()
            
    get_attr_names: ->
        []

    # called by set. change_level should not be defined by the user (it permits to != change from child of from this)
    __signal_change: ( change_level = 2 ) ->
        container = @__container()

        if container.__date_last_modification <= Model.__counter
            container.__date_last_modification = Model.__counter + change_level
            Model.__modlist.add container
            
            if container.__pardeps?
                for p in container.__pardeps
                    p.__signal_change 1
                
            # start a timer (if not already done)
            Model.__need_sync_views()

    #
    @__need_sync_views: ->
        if not Model.__timeout?
            Model.__timeout = setTimeout Model.__sync_views, 1

    # the function that is called after the (very short) timeout, when at least one object has been modified
    @__sync_views: ->
        # view -> forced onchange
        views = new Map 
        
        Model.__modlist.forEach ( model ) ->
            model.__views.forEach ( view ) ->
                views.set view, false

        for view in Model.__n_views
            views.set view, true

        # reset list of changed stiff
        Model.__timeout = undefined
        Model.__modlist = new Set
        Model.__n_views = []
        Model.__counter += 2
        
        # call onchange
        views.forEach ( force, view ) ->
            Model.__force_m = force
            view.onchange()
        Model.__force_m = false

    # __container -> get elder
    __container: ->
        if @__parent?
            return @__parent.__container()
        this
