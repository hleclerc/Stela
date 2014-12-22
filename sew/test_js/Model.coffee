class Model
    # static attributes
    @__counter: 0  # nb "change rounds" since the beginning ( * 2 to differenciate direct and indirect changes )
    @__modlist: new WeakSet # models changed models during the current round
    @__n_views: {} # new views (that will need a first onchange call in "force" mode)
    @__timeout: undefined # timer used to create a new "round" (to call View.on_change)
    @__force_m: false # if __force_m == true, every has_been_modified function will return true (used to call onchange of new views)
    @__id_map : new WeakMap # __id -> model
    @__cur_id : 1 # current model id (the one that will be used for the next created base model)
    
    constructor: ( @__ptr = asm_mod.allocate( 1 ), parent ) ->
        if parent?
            @__parent = parent

    # attr_names -> list with name of attributes. May be surdefined
    Object.defineProperty Model.prototype, "attr_names",
        get: () -> []

    #
    bind: ( f, onchange_construction = true ) ->
        if f instanceof View
            views = @__container.__views
            if views.indexOf( f ) < 0
                views.push view
                # f.__models.push this

            if onchange_construction
                Model.__n_views[ f.view_id ] = f
                Model._need_sync_views()
        else
            new FunctionBinder this, onchange_construction, f

    # called by set. change_level should not be defined by the user (it permits to != change from child of from this)
    _signal_change: ( change_level = 2 ) ->
        # register this as a modified model
        Model.__modlist[ @__id ] = this

        #
        if not @__orig.__modified_attributes[ @__numsub ]? or @__orig.__modified_attributes[ @__numsub ] < change_level
            @__orig.__modified_attributes[ @__numsub ] = change_level

        # do the same thing for the parents
        if @__orig.__date_last_modification <= Model.__counter
            @__orig.__date_last_modification = Model.__counter + change_level
            for p in @__orig.__parents
                p._signal_change 1
                
        # start if not done a timer
        Model._need_sync_views()

    # __container -> get elder
    Object.defineProperty Model.prototype, "__container",
        get: () ->
            if @parent?
                @parent.__container
            else
                @__add_mv_attr()
                this
    
    # __mv_attr -> add attributes needed for model/view manipulations
    __add_mv_attr: ( view ) ->
        if @__views?
            return
        @__views                  = []
        @__id                     = 0 # global by default (local if not saved in SocaDB)
        @__date_last_modification = 0 # Model.current_date+2 for a direct modification. Model.current_date+1 for a modification that comes from a child.

