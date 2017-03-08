# Real-Time Dynamic Global Illumination
This is the code for my master's thesis on real-time dynamic global illumination using the many-lights approach. The thesis itself can be found here: https://github.com/karyon/masterthesis, including a [compiled pdf](https://github.com/karyon/masterthesis/blob/master/thesis-final.pdf).

The following techniques have been implemented:

* Reflective Shadow Maps. They are rendered via the normal g-buffer shaders ([model.vert](data/shaders/model.vert), [model.frag](data/shaders/model.frag)) and regularly sampled in [vpl_processor.comp](data/shaders/gi/vpl_processor.comp)
* Imperfect Shadow Maps. The scene is converted to points with tessellation shaders ([ism.tesc](data/shaders/ism/ism.tesc), [ism.tese](data/shaders/ism/ism.tese)) and then rendered either via splatting ([ism.geom](data/shaders/ism/ism.geom), [ism.frag](data/shaders/ism/ism.frag)) or as single pixels via a compute shader ([ism.geom](data/shaders/ism/ism.geom), [ism.comp](data/shaders/ism/ism.comp)). In case of the single-pixel renderer, a pull-push postprocossing is applied ([pull.comp](data/shaders/ism/pull.comp), [push.comp](data/shaders/ism/push.comp)).
* Interleaved Sampling. This has been integrated into the final gathering shader. ([final_gathering.comp](data/shaders/gi/final_gathering.comp)). No buffers are split and re-interleaved; the result is pretty efficient.
* Clustered Deferred Shading. Implemented in two compute shader passes ([clustering.comp](data/shaders/clustered_shading/clustering.comp), [light_lists.comp](data/shaders/clustered_shading/light_lists.comp)). Turned out to have too much overhead in the contex of many-light methods.
* Tiled Deferred Shading. Integrated into the final gathering shader ([final_gathering.comp](https://github.com/karyon/many-lights-gi/blob/tiled_shading/data/shaders/gi/final_gathering.comp#L109-L174), this is in a separate branch) and a clear performance win in all test cases.

For a more thorough documentation see the implementation chapter in [the thesis](https://github.com/karyon/masterthesis/blob/master/thesis-final.pdf).

The end result is pretty fast (\<10ms for a complete frame, 6ms for the global illumination part, on a GTX 980), but the indirect shadows are of rather poor quality and are not temporally coherent, i.e. it flickers when lights or geometry moves. Again, for a more detailed analysis see the results chapter in [the thesis](https://github.com/karyon/masterthesis/blob/master/thesis-final.pdf).

## Dependencies

Sorry :(

* [Assimp 3.3.1](https://github.com/assimp/assimp/releases/tag/v3.3.1)
* [glbinding @ 72fae95f8939a5c253faf57bebd0915a105bc511](https://github.com/cginternals/glbinding/commit/72fae95f8939a5c253faf57bebd0915a105bc511)
* [glkernel @ 5518736f5c701bdc7d5ff230840ba87ebec97bc8](https://github.com/cginternals/glkernel/commit/5518736f5c701bdc7d5ff230840ba87ebec97bc8)
* [glm 0.9.7.4](https://github.com/g-truc/glm/releases/tag/0.9.7.4)
* [globjects @ 33bdd04396ab3a4b9270a9d944ef170ce7e96adc](https://github.com/cginternals/globjects/commit/33bdd04396ab3a4b9270a9d944ef170ce7e96adc)
* [gloperate (my fork) @ 6d2ae8ec46e44660fccb4f4fe32f1212c0b9550b](https://github.com/karyon/gloperate/commit/6d2ae8ec46e44660fccb4f4fe32f1212c0b9550b)
* [libzeug @ 52f7a419396653a3dd05a5a43231af59d9b5051c](https://github.com/cginternals/libzeug/commit/52f7a419396653a3dd05a5a43231af59d9b5051c)
* [Qt](https://www.qt.io/) 5.6
