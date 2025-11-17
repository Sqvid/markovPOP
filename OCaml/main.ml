let max_output = 200

let words =
  Str.split (Str.regexp "[ \t\r\n]+") (In_channel.input_all In_channel.stdin)

module Prefix = struct
  type t = string * string

  let compare (s1, s2) (s3, s4) = String.compare (s1 ^ s2) (s3 ^ s4)
end

module PrefixMap = Map.Make (Prefix)

let build_state_map =
  let rec build_state_map' (map : string option Dynarray.t PrefixMap.t)
      (words : string list) =
    match words with
    | w1 :: w2 :: t -> begin
        let prefix = (w1, w2) in
        let w3 = if t = [] then None else Some (List.hd t) in
        let str_arr =
          match PrefixMap.find_opt prefix map with
          | None -> Dynarray.make 1 w3
          | Some arr ->
              Dynarray.add_last arr w3;
              arr
        in
        let map' = PrefixMap.add prefix str_arr map in
        build_state_map' map' (List.tl words)
      end
    | _ -> map
  in
  build_state_map' PrefixMap.empty

let () = Random.self_init ()

let make_prefix = function
  | w1 :: w2 :: _ -> (w1, w2)
  | _ -> raise (Invalid_argument "Input shorter than prefix length")

let generate n =
  let start_prefix = make_prefix words in
  let () = Printf.printf "%s %s " (fst start_prefix) (snd start_prefix) in
  let rec generate' n state_map prefix =
    if n = 0 then print_newline ()
    else
      match PrefixMap.find_opt prefix state_map with
      | None -> print_newline ()
      | Some dynArr -> begin
          let idx = Random.int (Dynarray.length dynArr) in
          match Dynarray.get dynArr idx with
          | None -> print_newline ()
          | Some str ->
              Printf.printf "%s " str;
              generate' (n - 1) state_map (snd prefix, str)
        end
  in
  generate' n (build_state_map words) start_prefix

let () = generate max_output
