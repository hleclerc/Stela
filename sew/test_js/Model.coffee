class Model
    # static attributes
    @__counter: 0  # nb "change rounds" since the beginning ( * 2 to differenciate direct and indirect changes )
    @__modlist: [] # changed models (current round)
    @__n_views: {} # new views (that will need a first onchange call in "force" mode)
    @__timeout: undefined # timer used to create a new "round" (to call View.on_change)
    @__force_m: false # if __force_m == true, every has_been_modified function will return true (used to call onchange of new views)
    @__id_map : new WeakMap() # __id -> model
    @__cur_id : 1 # current model id (the one that will be used for the next created base model)
    
    constructor: ( @__ptr = asm_mod.allocate( 1 ), parent ) ->
        if parent?
            @__parent = parent

    # attr_names -> list with name of attributes. May be surdefined
    Object.defineProperty Model.prototype, "attr_names",
        get: () -> []

    #
    bind: ( view ) ->
        views = @__container.__views
        if views.indexOf( view ) < 0
            views.push view


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

